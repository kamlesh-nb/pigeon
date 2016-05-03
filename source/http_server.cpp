
#include <http_server.h>
#include <http_context.h>
#include <settings.h>
#include <http_parser.h>
#include <uv.h>
#include <malloc.h>
#include <logger.h>
#include <cache.h>
#include <http_handlers.h>
#include <resource_handler.h>

using namespace pigeon;

#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

string IPC_PIPE_NAME;

 
http_parser_settings parser_settings;

typedef struct {
	uv_tcp_t handle;
	uv_shutdown_t shutdown_req;
} conn_rec_t;

typedef struct {

	uv_tcp_t stream;
	http_parser parser;
	uv_write_t write_req;
	char *client_address;
	void *data;
	char *temp;
	http_context *context;

} iconnection_t;


typedef struct {

	uv_work_t request;
	iconnection_t* iConn;
	bool error;
	char* result;
	size_t length;

} msg_baton_t;

union stream_handle {
	uv_pipe_t pipe;
	uv_tcp_t tcp;
};

typedef unsigned char handle_storage_t[sizeof(union stream_handle)];


struct ipc_server_ctx {
	handle_storage_t server_handle;
	unsigned int num_connects;
	uv_pipe_t ipc_pipe;
};

struct ipc_peer_ctx {
	handle_storage_t peer_handle;
	uv_write_t write_req;
};

struct ipc_client_ctx {
	uv_connect_t connect_req;
	uv_stream_t* server_handle;
	uv_pipe_t ipc_pipe;
	char scratch[16];
};

struct server_ctx {
	uv_loop_t* loop;
	handle_storage_t server_handle;
	unsigned int num_connects;
	uv_async_t async_handle;
	uv_thread_t thread_id;
	uv_sem_t semaphore;
	void* tcp_ptr;
	void* pipe_ptr;
};

struct sockaddr_in listen_addr;
uv_async_t* listener_async_handles;
uv_barrier_t* listeners_created_barrier;
uv_tcp_t server;

class http_server::pipe {

private:


public:

	void send_listen_handles(uv_handle_type type, unsigned int num_servers, struct server_ctx* servers) {
		int r;
		struct ipc_server_ctx ctx;
		uv_loop_t* loop;
		unsigned int i;

		loop = uv_default_loop();
		ctx.num_connects = num_servers;
		ctx.ipc_pipe.data = servers[0].pipe_ptr;

		if (type == UV_TCP) {
			r = uv_tcp_init(loop, (uv_tcp_t*)&ctx.server_handle);
			r = uv_tcp_bind((uv_tcp_t*)&ctx.server_handle, (const struct sockaddr*) &listen_addr, 0);
		}

		r = uv_pipe_init(loop, &ctx.ipc_pipe, 1);
		r = uv_pipe_bind(&ctx.ipc_pipe, IPC_PIPE_NAME.c_str());
		r = uv_listen((uv_stream_t*)&ctx.ipc_pipe, 128, [](uv_stream_t* ipc_pipe, int status) {
			pipe* pipe_ptr = reinterpret_cast<pipe*>(ipc_pipe->data);
			pipe_ptr->ipc_connection_cb(ipc_pipe, status);
		});

		for (i = 0; i < num_servers; i++)
			uv_sem_post(&servers[i].semaphore);

		r = uv_run(loop, UV_RUN_DEFAULT);
		uv_close((uv_handle_t*)&ctx.server_handle, NULL);
		r = uv_run(loop, UV_RUN_DEFAULT);

		for (i = 0; i < num_servers; i++)
			uv_sem_wait(&servers[i].semaphore);

	}


	void get_listen_handle(uv_loop_t* loop, uv_stream_t* server_handle) {
		int r;
		struct ipc_client_ctx ctx;
		ctx.server_handle = server_handle;
		ctx.server_handle->data = (void*)"server handle";
		ctx.connect_req.data = this;
		r = uv_pipe_init(loop, &ctx.ipc_pipe, 1);
		uv_pipe_connect(&ctx.connect_req, &ctx.ipc_pipe, IPC_PIPE_NAME.c_str(), 
		[](uv_connect_t* req, int status) {
			pipe* pipe_ptr = reinterpret_cast<pipe*>(req->data);
			pipe_ptr->ipc_connect_cb(req, status);
		});
		r = uv_run(loop, UV_RUN_DEFAULT);

	}

	 

	void ipc_connection_cb(uv_stream_t* ipc_pipe, int status) {
		int r;
		struct ipc_server_ctx* sc;
		struct ipc_peer_ctx* pc;
		uv_loop_t* loop;
		uv_buf_t buf;

		loop = ipc_pipe->loop;
		buf = uv_buf_init("PING", 4);
		sc = container_of(ipc_pipe, struct ipc_server_ctx, ipc_pipe);
		pc = (struct ipc_peer_ctx*)calloc(1, sizeof(*pc));

		pc->write_req.data = this;

		if (ipc_pipe->type == UV_TCP)
			r = uv_tcp_init(loop, (uv_tcp_t*)&pc->peer_handle);
		else if (ipc_pipe->type == UV_NAMED_PIPE)
			r = uv_pipe_init(loop, (uv_pipe_t*)&pc->peer_handle, 1);

		r = uv_accept(ipc_pipe, (uv_stream_t*)&pc->peer_handle);
		r = uv_write2(&pc->write_req, (uv_stream_t*)&pc->peer_handle, &buf, 1, (uv_stream_t*)&sc->server_handle, 
			[](uv_write_t* req, int status) {
				pipe* pipe_ptr = reinterpret_cast<pipe*>(req->data);
				pipe_ptr->ipc_write_cb(req, status);
			});

		if (--sc->num_connects == 0)
			uv_close((uv_handle_t*)ipc_pipe, NULL);

	}



	void ipc_write_cb(uv_write_t* req, int status) {
		struct ipc_peer_ctx* ctx;
		ctx = container_of(req, struct ipc_peer_ctx, write_req);
		uv_close((uv_handle_t*)&ctx->peer_handle, [](uv_handle_t* handle) {
			pipe* pipe_ptr = reinterpret_cast<pipe*>(handle->data);
			pipe_ptr->ipc_close_cb(handle);
		});
	}

	void ipc_close_cb(uv_handle_t* handle) {
		struct ipc_peer_ctx* ctx;
		ctx = container_of(handle, struct ipc_peer_ctx, peer_handle);
		free(ctx);
	}

	void ipc_connect_cb(uv_connect_t* req, int status) {
		int r;
		struct ipc_client_ctx* ctx;
		ctx = container_of(req, struct ipc_client_ctx, connect_req);
		ctx->ipc_pipe.data = this;

		r = uv_read_start((uv_stream_t*)&ctx->ipc_pipe, [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
			pipe* pipe_ptr = reinterpret_cast<pipe*>(handle->data);
			pipe_ptr->ipc_alloc_cb(handle, suggested_size, buf);
		}, [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
			pipe* pipe_ptr = reinterpret_cast<pipe*>(handle->data);
			pipe_ptr->ipc_read_cb(handle, nread, buf);
		});

	}

 

	void ipc_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
		struct ipc_client_ctx* ctx;
		ctx = container_of(handle, struct ipc_client_ctx, ipc_pipe);
		buf->base = ctx->scratch;
		buf->len = sizeof(ctx->scratch);
	}

	void ipc_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
		int r;
		struct ipc_client_ctx* ctx;
		uv_loop_t* loop;
		uv_handle_type type;
		uv_pipe_t* ipc_pipe;

		ipc_pipe = (uv_pipe_t*)handle;
		ctx = container_of(ipc_pipe, struct ipc_client_ctx, ipc_pipe);
		loop = ipc_pipe->loop;

		r = uv_pipe_pending_count(ipc_pipe);
		type = uv_pipe_pending_type(ipc_pipe);

		if (type == UV_TCP)
			r = uv_tcp_init(loop, (uv_tcp_t*)ctx->server_handle);
		else if (type == UV_NAMED_PIPE)
			r = uv_pipe_init(loop, (uv_pipe_t*)ctx->server_handle, 0);

		r = uv_accept(handle, ctx->server_handle);
		uv_close((uv_handle_t*)&ctx->ipc_pipe, NULL);
	}


};


class http_server::tcp {

private:

public:

	void server_cb(void *arg) {

		int r;
		struct server_ctx *ctx;
		uv_loop_t* loop;

		ctx = (struct server_ctx *)arg;
		loop = uv_loop_new();
		
		ctx->loop = loop;
		http_server::pipe* pipe_ptr = reinterpret_cast<http_server::pipe*>(ctx->pipe_ptr);
		uv_barrier_wait(listeners_created_barrier);

		r = uv_async_init(loop, &ctx->async_handle, 
			[](uv_async_t* handle) {
				struct server_ctx* ctx;
				ctx = container_of(handle, struct server_ctx, async_handle);
				uv_close((uv_handle_t*)&ctx->server_handle, NULL);
				uv_close((uv_handle_t*)&ctx->async_handle, NULL);
			});

		uv_unref((uv_handle_t*)&ctx->async_handle);
		
		uv_sem_wait(&ctx->semaphore);
		pipe_ptr->get_listen_handle(loop, (uv_stream_t*)&ctx->server_handle);
		uv_sem_post(&ctx->semaphore);

		r = uv_listen((uv_stream_t*)&ctx->server_handle, 128, 
			[](uv_stream_t* handle, int status) {
				struct server_ctx* ctx;
				ctx = container_of(handle, struct server_ctx, server_handle);
				http_server::tcp*  tcp_ptr = reinterpret_cast<http_server::tcp*>(ctx->tcp_ptr);
				tcp_ptr->sv_connection_cb(handle, status);
			});
		r = uv_run(loop, UV_RUN_DEFAULT);

		uv_loop_delete(loop);

	}

	void sv_async_cb(uv_async_t* handle) {
		struct server_ctx* ctx;
		ctx = container_of(handle, struct server_ctx, async_handle);
		uv_close((uv_handle_t*)&ctx->server_handle, NULL);
		uv_close((uv_handle_t*)&ctx->async_handle, NULL);
	}

	void sv_connection_cb(uv_stream_t* server_handle, int status) {

		int r;
		iconnection_t* iConn = (iconnection_t*)malloc(sizeof(iconnection_t));
		iConn->context = new http_context;

		iConn->parser.data = iConn;
		iConn->stream.data = iConn;

		r = uv_tcp_init(server_handle->loop, &iConn->stream);
		http_parser_init(&iConn->parser, HTTP_REQUEST);

		r = uv_accept(server_handle, (uv_stream_t*)&iConn->stream);
		r = uv_read_start((uv_stream_t*)&iConn->stream, 
			[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
			buf->base = (char*)malloc(suggested_size);
			buf->len = suggested_size;
			}, 
			[](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
				ssize_t parsed;
				iconnection_t *iConn = (iconnection_t *)handle->data;
				if (nread >= 0) {
					parsed = (ssize_t)http_parser_execute(&iConn->parser, &parser_settings, buf->base, nread);
					if (parsed < nread) {
						uv_close((uv_handle_t *)&iConn->stream, [](uv_handle_t *handle) {
							iconnection_t* iConn = (iconnection_t*)handle->data;
							delete iConn->context;
							free(iConn);
						});
					}
				}
				else {
					if (nread != UV_EOF) {
					}
					uv_close((uv_handle_t *)&iConn->stream, [](uv_handle_t *handle) {
						iconnection_t* iConn = (iconnection_t*)handle->data;
						delete iConn->context;
						free(iConn);
					});
				}
				free(buf->base);
		
			});

	}

	void sv_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
		buf->base = (char*)malloc(suggested_size);
		buf->len = suggested_size;
	}

	auto on_close(uv_handle_t *handle) -> void {
 
		iconnection_t* iConn = (iconnection_t*)handle->data;
		delete iConn->context;
		free(iConn);

	}

	auto on_shutdown(uv_shutdown_t* req, int status) -> void {

		conn_rec_t* conn = container_of(req, conn_rec_t, shutdown_req);
		msg_baton_t *closure = static_cast<msg_baton_t *>(req->data);
		delete closure;
		uv_close((uv_handle_t*)&conn->handle, [](uv_handle_t *handle) {
			iconnection_t* iConn = (iconnection_t*)handle->data;
			delete iConn->context;
			free(iConn);
		});

	}

	void sv_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
		 

			ssize_t parsed;
			iconnection_t *iConn = (iconnection_t *)handle->data;
			if (nread >= 0) {
				parsed = (ssize_t)http_parser_execute(&iConn->parser, &parser_settings, buf->base, nread);
				if (parsed < nread) {
					uv_close((uv_handle_t *)&iConn->stream, [](uv_handle_t *handle) {
						iconnection_t* iConn = (iconnection_t*)handle->data;
						delete iConn->context;
						free(iConn);
					});
				}
			}
			else {
				if (nread != UV_EOF) {
				}
				uv_close((uv_handle_t *)&iConn->stream, [](uv_handle_t *handle) {
					iconnection_t* iConn = (iconnection_t*)handle->data;
					delete iConn->context;
					free(iConn);
				});
			}
			free(buf->base);
		 
	}

};



http_server::http_server()
{
	tcpImpl = new tcp;
	pipeImpl = new pipe;
}

http_server::~http_server()
{
}

void http_server::_parser() {

	parser_settings.on_url = [](http_parser *parser, const char *at, size_t len) -> int {

		iconnection_t *iConn = (iconnection_t *)parser->data;
		if (at && iConn->context->request) {
			char *data = (char *)malloc(sizeof(char) * len + 1);
			strncpy(data, at, len);
			data[len] = '\0';
			iConn->context->request->url += data;
			free(data);
		}
		return 0;

	};

	parser_settings.on_header_field = [](http_parser *parser, const char *at, size_t len) -> int {

		iconnection_t *iConn = (iconnection_t *)parser->data;
		if (at && iConn->context->request) {
			string s;
			iConn->temp = (char *)malloc(sizeof(char) * len + 1);

			strncpy(iConn->temp, at, len);
			iConn->temp[len] = '\0';

		}
		return 0;

	};

	parser_settings.on_header_value = [](http_parser *parser, const char *at, size_t len) -> int {

		iconnection_t *iConn = (iconnection_t *)parser->data;
		if (at && iConn->context->request) {
			string key, value;
			char *data = (char *)malloc(sizeof(char) * len + 1);
			strncpy(data, at, len);
			data[len] = '\0';
			value += data;
			key += iConn->temp;
			free(data);
			free(iConn->temp);

			iConn->context->request->set_header(key, value);
		}
		return 0;

	};

	parser_settings.on_headers_complete = [](http_parser *parser) -> int {

		iconnection_t *iConn = (iconnection_t *)parser->data;
		iConn->context->request->method = parser->method;
		iConn->context->request->http_major_version = parser->http_major;
		iConn->context->request->http_minor_version = parser->http_minor;
		return 0;

	};

	parser_settings.on_body = [](http_parser *parser, const char *at, size_t len) -> int {

		iconnection_t *iConn = (iconnection_t *)parser->data;
		if (at && iConn->context->request) {
			// const char* end = at + len;
			// iConn->context->request->content.insert(iConn->context->request->content.begin(),
			//     at, end);

			for (size_t i = 0; i < len; ++i) {
				iConn->context->request->content.push_back(at[i]);
			}

		}
		return 0;

	};

	parser_settings.on_message_complete = [](http_parser *parser) -> int {

		iconnection_t *iConn = (iconnection_t *)parser->data;
		msg_baton_t *closure = new msg_baton_t();
		closure->request.data = closure;
		closure->iConn = iConn;
		closure->error = false;

	 

	/*	if (status != 0) {
			logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(status));
		}
*/
		return 0;

	};

}

void http_server::_init()
{
	uv_async_t* service_handle = 0;

	struct server_ctx* servers;
	struct client_ctx* clients;
	uv_loop_t* loop;

	unsigned int i;
	double time;
	int r;

	int port = settings::port;
	string addr = settings::address;
	unsigned int no_of_threads = settings::worker_threads;
	r = uv_ip4_addr(addr.c_str(), port, &listen_addr);
	
	server.data = tcpImpl;

	loop = uv_default_loop();
	uv_tcp_init(loop, &server);

	listener_async_handles = (uv_async_t*)calloc(no_of_threads, sizeof(uv_async_t));

	listeners_created_barrier = (uv_barrier_t*)malloc(sizeof(uv_barrier_t));
	uv_barrier_init(listeners_created_barrier, no_of_threads + 1);

	service_handle = (uv_async_t*)malloc(sizeof(uv_async_t));
	uv_async_init(loop, service_handle, NULL);


	servers = (struct server_ctx*)calloc(no_of_threads, sizeof(servers[0]));

	for (i = 0; i < no_of_threads; i++) {
		struct server_ctx* ctx = servers + i;
		ctx->tcp_ptr = tcpImpl;
		ctx->pipe_ptr = pipeImpl;

		r = uv_sem_init(&ctx->semaphore, 0);
		//r = uv_thread_create(&ctx->thread_id, server_cb, ctx);
		r = uv_thread_create(&ctx->thread_id, [](void* arg) {
			struct server_ctx *ctx;
			ctx = (struct server_ctx *)arg;
			tcp* impl = reinterpret_cast<tcp*>(ctx->tcp_ptr);
			impl->server_cb(ctx);
		}, ctx);
	}
	uv_barrier_wait(listeners_created_barrier);

	pipeImpl->send_listen_handles(UV_TCP, no_of_threads, servers);
	uv_run(loop, UV_RUN_DEFAULT);

}

void http_server::run()
{

	settings::load_setting();
	cache::get()->load(settings::resource_location);

	http_handlers::instance()->add("resource", new resource_handler());

	RequestProcessor = new request_processor();
	_parser();


#ifdef _WIN32
	IPC_PIPE_NAME += "\\\\?\\pipe\\";
	IPC_PIPE_NAME += settings::service_name;
#else
	IPC_PIPE_NAME += "/tmp/";
	IPC_PIPE_NAME += _Settings->get_service_name();
#endif
	
	_init();
}
