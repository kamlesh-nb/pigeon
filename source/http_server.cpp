

#include <malloc.h>

#include "http_server.h"
#include "http_context.h"
#include "settings.h"
#include "http_parser.h"
#include "logger.h"
#include "cache.h"
#include "http_handlers.h"
#include "resource_handler.h"
#include "request_processor.h"


using namespace pigeon;

#define LOG(r) \
if (r) { \
logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r)); \
exit(1); \
}

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
    void* rp_ptr;
	char *temp;
	http_context *context;

} client_t;

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
	void* ptr;
    void* rp_ptr;
};

struct sockaddr_in listen_addr;
uv_async_t* listener_async_handles;
uv_barrier_t* listeners_created_barrier;
uv_tcp_t server;

class http_server::http_server_impl {

private:

	request_processor* RequestProcessor;
	/*setup htp_parser callback lambda's*/
	void parser() {

		parser_settings.on_url = [](http_parser *parser, const char *at, size_t len) -> int {

			client_t *client = (client_t *)parser->data;
			if (at && client->context->request) {
				char *data = (char *)malloc(sizeof(char) * len + 1);
				strncpy(data, at, len);
				data[len] = '\0';
				client->context->request->url += data;
				free(data);
			}
			return 0;

		};

		parser_settings.on_header_field = [](http_parser *parser, const char *at, size_t len) -> int {

			client_t *client = (client_t *)parser->data;
			if (at && client->context->request) {
				string s;
				client->temp = (char *)malloc(sizeof(char) * len + 1);

				strncpy(client->temp, at, len);
				client->temp[len] = '\0';

			}
			return 0;

		};

		parser_settings.on_header_value = [](http_parser *parser, const char *at, size_t len) -> int {

			client_t *client = (client_t *)parser->data;
			if (at && client->context->request) {
				string key, value;
				char *data = (char *)malloc(sizeof(char) * len + 1);
				strncpy(data, at, len);
				data[len] = '\0';
				value += data;
				key += client->temp;
				free(data);
				free(client->temp);

				client->context->request->set_header(key, value);
			}
			return 0;

		};

		parser_settings.on_headers_complete = [](http_parser *parser) -> int {

			client_t *client = (client_t *)parser->data;
			client->context->request->method = parser->method;
			client->context->request->http_major_version = parser->http_major;
			client->context->request->http_minor_version = parser->http_minor;
			return 0;

		};

		parser_settings.on_body = [](http_parser *parser, const char *at, size_t len) -> int {

			client_t *client = (client_t *)parser->data;
			if (at && client->context->request) {
				for (size_t i = 0; i < len; ++i) {
					client->context->request->content.push_back(at[i]);
				}

			}
			return 0;

		};

		parser_settings.on_message_complete = [](http_parser *parser) -> int {

			client_t *client = (client_t *)parser->data;
            client->context->data = client;

			request_processor* rp = reinterpret_cast<request_processor*>(client->rp_ptr);
			rp->process(client->context, [](http_context* context){

                client_t* client = (client_t*)context->data;
                uv_buf_t resbuf;
                resbuf.base = (char*)client->context->response->buffer->to_cstr();
                resbuf.len = (unsigned long)client->context->response->buffer->get_length();

                client->write_req.data = client;
                int r = uv_write(&client->write_req,
                                 (uv_stream_t*)&client->stream,
                                 &resbuf,
                                 1,
                                 [](uv_write_t *req, int status) {

                                     LOG(status);

                                     if (!uv_is_closing((uv_handle_t*)req->handle))
                                     {
                                         uv_close((uv_handle_t*)req->handle,
                                                  [](uv_handle_t *handle) {
                                                      client_t* client = (client_t*)handle->data;
                                                      client->context->response->buffer->clear();
                                                      delete client->context->response->buffer;
                                                      delete client->context;
                                                      free(client);
                                                  });
                                     }


                                 });

                LOG(r);

            });
			return 0;

		};

	}



	/*initialize the server*/
	void init()
	{
		uv_async_t* service_handle = 0;

#ifndef _WIN32
		signal(SIGPIPE, SIG_IGN);
#endif
		 
		uv_loop_t* loop;

		unsigned int i;
		int r;

		
		unsigned int threads = settings::worker_threads;

		server.data = this;

		loop = uv_default_loop();
		uv_tcp_init(loop, &server);

		listener_async_handles = (uv_async_t*)calloc(threads, sizeof(uv_async_t));

		listeners_created_barrier = (uv_barrier_t*)malloc(sizeof(uv_barrier_t));
		uv_barrier_init(listeners_created_barrier, threads + 1);

		service_handle = (uv_async_t*)malloc(sizeof(uv_async_t));
		uv_async_init(loop, service_handle, NULL);



		struct server_ctx* servers;
		servers = (struct server_ctx*)calloc(threads, sizeof(servers[0]));

		for (i = 0; i < threads; i++) {
			struct server_ctx* ctx = servers + i;
			ctx->ptr = this;
            request_processor* rp = new request_processor;
            ctx->rp_ptr = rp;
			r = uv_sem_init(&ctx->semaphore, 0);
            LOG(r);
			r = uv_thread_create(&ctx->thread_id, [](void* arg) {
				struct server_ctx *ctx;
				ctx = (struct server_ctx *)arg;
				http_server::http_server_impl* _Impl = reinterpret_cast<http_server::http_server_impl*>(ctx->ptr);
				_Impl->process(ctx);
			}, ctx);
            LOG(r);
		}

		uv_barrier_wait(listeners_created_barrier);
		dispatch_listen_handles(UV_TCP, threads, servers);
		uv_run(loop, UV_RUN_DEFAULT);

	}

public:

	/*process the new connections*/
	void process(void *arg) {

		int r;
		struct server_ctx *ctx;
		uv_loop_t* loop;

		ctx = (struct server_ctx *)arg;
		loop = uv_loop_new();

		ctx->loop = loop;

		uv_barrier_wait(listeners_created_barrier);

		r = uv_async_init(loop, &ctx->async_handle,
			/*sv_async_cb*/
			[](uv_async_t* handle) {
				struct server_ctx* ctx;
				ctx = container_of(handle, struct server_ctx, async_handle);
				uv_close((uv_handle_t*)&ctx->server_handle, NULL);
				uv_close((uv_handle_t*)&ctx->async_handle, NULL);
			});
			/*sv_async_cb*/

        LOG(r);

		uv_unref((uv_handle_t*)&ctx->async_handle);

		uv_sem_wait(&ctx->semaphore);
		receive_listen_handle(loop, (uv_stream_t*)&ctx->server_handle);
		uv_sem_post(&ctx->semaphore);

		r = uv_listen((uv_stream_t*)&ctx->server_handle, 128,
			/*sv_connection_cb*/
			[](uv_stream_t* server_handle, int status) {

                LOG(status);

                int r;

                struct server_ctx* ctx;
                ctx = container_of(server_handle, struct server_ctx, server_handle);

				client_t* client = (client_t*)malloc(sizeof(client_t));
				client->context = new http_context;
				client->parser.data = client;
				client->stream.data = client;
				client->data = ((uv_stream_t*)server_handle)->data;
                client->rp_ptr = ctx->rp_ptr;

				r = uv_tcp_init(server_handle->loop, &client->stream);
                if (settings::tcp_no_delay) {
                    r = uv_tcp_nodelay((uv_tcp_t*)&client->stream, 1);
                }
				http_parser_init(&client->parser, HTTP_REQUEST);

				r = uv_accept(server_handle, (uv_stream_t*)&client->stream);
                LOG(r);
				r = uv_read_start((uv_stream_t*)&client->stream,
					/*sv_alloc_cb*/
					[](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
						buf->base = (char*)malloc(suggested_size);
						buf->len = suggested_size;
					}
					/*sv_alloc_cb*/
					,
					/*sv_read_cb*/
					[](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
						ssize_t parsed;
						client_t *client = (client_t *)handle->data;
						if (nread >= 0) {
							parsed = (ssize_t)http_parser_execute(&client->parser, &parser_settings, buf->base, nread);
							if (parsed < nread) {
								//use logger to log error
								uv_close((uv_handle_t *)&client->stream,
									/*on_close*/
									[](uv_handle_t *handle) {
										client_t* client = (client_t*)handle->data;
										delete client->context;
										free(client);
									});
									/*on_close*/
							}
						}
						else {
							if (nread != UV_EOF) {
								//use logger to log error
							}
							uv_close((uv_handle_t *)&client->stream,
								/*on_close*/
								[](uv_handle_t *handle) {
									client_t* client = (client_t*)handle->data;
									delete client->context;
									free(client);
								});
								/*on_close*/
						}

						free(buf->base);
					});
					/*sv_read_cb*/
                LOG(r);

		});
        LOG(r);
		/*sv_connection_cb*/
		r = uv_run(loop, UV_RUN_DEFAULT);
        LOG(r);

		uv_loop_delete(loop);

	}

	void dispatch_listen_handles(uv_handle_type type, unsigned int num_servers, struct server_ctx* servers) {
		int r;
		struct ipc_server_ctx ctx;
		uv_loop_t* loop;
		unsigned int i;

		loop = uv_default_loop();
		ctx.num_connects = num_servers;

		if (type == UV_TCP) {

			int port = settings::port;
			string addr = settings::address;
			r = uv_ip4_addr(addr.c_str(), port, &listen_addr);
            LOG(r);
			r = uv_tcp_init(loop, (uv_tcp_t*)&ctx.server_handle);
            LOG(r);
			if (settings::tcp_no_delay) {
				r = uv_tcp_nodelay((uv_tcp_t*)&ctx.server_handle, 1);
                LOG(r);
			}
			r = uv_tcp_bind((uv_tcp_t*)&ctx.server_handle, (const struct sockaddr*) &listen_addr, 0);
            LOG(r);
		}

		r = uv_pipe_init(loop, &ctx.ipc_pipe, 1);
        LOG(r);
		r = uv_pipe_bind(&ctx.ipc_pipe, IPC_PIPE_NAME.c_str());
        LOG(r);
		r = uv_listen((uv_stream_t*)&ctx.ipc_pipe, 128,
			/*ipc_connection_cb*/
			[](uv_stream_t* ipc_pipe, int status) {

                LOG(status);

				int rc;
				struct ipc_server_ctx* sc;
				struct ipc_peer_ctx* pc;
				uv_loop_t* loop;
				uv_buf_t buf;

				loop = ipc_pipe->loop;
                const char* ping = "PING";
				buf = uv_buf_init((char*)ping, 4);
				sc = container_of(ipc_pipe, struct ipc_server_ctx, ipc_pipe);
				pc = (struct ipc_peer_ctx*)calloc(1, sizeof(*pc));

				if (ipc_pipe->type == UV_TCP) {
					rc = uv_tcp_init(loop, (uv_tcp_t*)&pc->peer_handle);
                    LOG(rc);
					if (settings::tcp_no_delay) {
						rc = uv_tcp_nodelay((uv_tcp_t*)&pc->peer_handle, 1);
                        LOG(rc);
					}
				}
				else if (ipc_pipe->type == UV_NAMED_PIPE) {
                    rc = uv_pipe_init(loop, (uv_pipe_t *) &pc->peer_handle, 1);
                    LOG(rc);
                }
				rc = uv_accept(ipc_pipe, (uv_stream_t*)&pc->peer_handle);
                LOG(rc);
				rc = uv_write2(&pc->write_req,
					(uv_stream_t*)&pc->peer_handle,
					&buf,
					1,
					(uv_stream_t*)&sc->server_handle,
					/*ipc_write_cb*/
					[](uv_write_t* req, int status) {

                        if (status != 0) {
                            logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(status));
                        }
						struct ipc_peer_ctx* ctx;
						ctx = container_of(req, struct ipc_peer_ctx, write_req);
						uv_close((uv_handle_t*)&ctx->peer_handle,
							/*ipc_close_cb*/
							[](uv_handle_t* handle) {
								struct ipc_peer_ctx* ctx;
								ctx = container_of(handle, struct ipc_peer_ctx, peer_handle);
								free(ctx);
							});
							/*ipc_close_cb*/
					});
					/*ipc_write_cb*/
                LOG(rc);

				if (--sc->num_connects == 0)
					uv_close((uv_handle_t*)ipc_pipe, NULL);

			});
			/*ipc_connection_cb*/
        LOG(r);

		for (i = 0; i < num_servers; i++)
			uv_sem_post(&servers[i].semaphore);

		r = uv_run(loop, UV_RUN_DEFAULT);
        LOG(r);
		uv_close((uv_handle_t*)&ctx.server_handle, NULL);
		r = uv_run(loop, UV_RUN_DEFAULT);
        LOG(r);

		for (i = 0; i < num_servers; i++)
			uv_sem_wait(&servers[i].semaphore);

	}

	void receive_listen_handle(uv_loop_t* loop, uv_stream_t* server_handle) {
		int r;
		struct ipc_client_ctx ctx;
		ctx.server_handle = server_handle;
		ctx.server_handle->data = (void*)"server handle";

		r = uv_pipe_init(loop, &ctx.ipc_pipe, 1);
        LOG(r);
		uv_pipe_connect(&ctx.connect_req, &ctx.ipc_pipe, IPC_PIPE_NAME.c_str(),
			/*ipc_connect*/
			[](uv_connect_t* req, int status) {
                LOG(status);

				int rc;
				struct ipc_client_ctx* ctx;
				ctx = container_of(req, struct ipc_client_ctx, connect_req);
				rc = uv_read_start((uv_stream_t*)&ctx->ipc_pipe,
					/*ipc_alloc*/
					[](uv_handle_t* handle, size_t /*suggested_size*/, uv_buf_t* buf) {
						struct ipc_client_ctx* ctx;

                        ctx = container_of(handle, struct ipc_client_ctx, ipc_pipe);
						buf->base = ctx->scratch;
						buf->len = sizeof(ctx->scratch);
					},
					/*ipc_read*/
					[](uv_stream_t* handle, ssize_t /*nread*/, const uv_buf_t* /*buf*/) {
						int r;
						struct ipc_client_ctx* ctx;
						uv_loop_t* loop;
						uv_handle_type type;
						uv_pipe_t* ipc_pipe;

						ipc_pipe = (uv_pipe_t*)handle;
						ctx = container_of(ipc_pipe, struct ipc_client_ctx, ipc_pipe);
						loop = ipc_pipe->loop;

						r = uv_pipe_pending_count(ipc_pipe);
                        LOG(r);
						type = uv_pipe_pending_type(ipc_pipe);

						if (type == UV_TCP) {
							r = uv_tcp_init(loop, (uv_tcp_t*)ctx->server_handle);
                            LOG(r);
							if (settings::tcp_no_delay) {
								r = uv_tcp_nodelay((uv_tcp_t*)ctx->server_handle, 1);
                                LOG(r);
							}
						}
						else if (type == UV_NAMED_PIPE) {
                            r = uv_pipe_init(loop, (uv_pipe_t *) ctx->server_handle, 0);
                            LOG(r);
                        }

						r = uv_accept(handle, ctx->server_handle);
                        LOG(r);
						uv_close((uv_handle_t*)&ctx->ipc_pipe, NULL);
			});
            LOG(rc);

		});
		r = uv_run(loop, UV_RUN_DEFAULT);
        LOG(r);

	}

	void run()
	{

		settings::load_setting();
		cache::get()->load(settings::resource_location);

		http_handlers::instance()->add("resource", new resource_handler());
		RequestProcessor = new request_processor();

		parser();


#ifdef _WIN32
		IPC_PIPE_NAME += "\\\\?\\pipe\\";
		IPC_PIPE_NAME += settings::service_name;
#else
		IPC_PIPE_NAME += "/tmp/";
		IPC_PIPE_NAME += settings::service_name;
#endif

		init();
	}

};



http_server::http_server()
{
	_Impl = new http_server_impl;
}

http_server::~http_server()
{
}

void http_server::start()
{
	_Impl->run();
}



