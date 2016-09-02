//
// Created by root on 9/1/16.
//

#include "tcp_listener.h"
#include <assert.h>

#define UV_ERR_TCP(msg, code) do {                                                         \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);


typedef struct {
	uv_tcp_t stream;
	uv_write_t write_req;
	char *client_address;
	void *data;
	void* rp_ptr;
	char *temp;
} client_t;

tcp_listener::tcp_listener()
{
}


tcp_listener::~tcp_listener()
{
}

void tcp_listener::get_handle(uv_loop_t* loop){

	uv_pipe_init(loop, ipc_pipe, 1);
	uv_pipe_connect(conn_req,
		ipc_pipe,
		pipe_name.c_str(),
		[](uv_connect_t* req, int status){
		tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(req->data);
		tcp_listener_ptr->on_ipc_connect(req, status);
	});
}

void tcp_listener::create(uv_barrier_t* p_barrier){
	barrier = p_barrier;
	pipe_name = "/tmp/uv";
	t = std::thread(&tcp_listener::start, this);
}

void tcp_listener::post(){
	uv_sem_post(semaphore);
}
void tcp_listener::wait(){
	uv_sem_wait(semaphore);
}

void tcp_listener::start(){

	int r;

	loop = uv_loop_new();

	ipc_pipe = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
	conn_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
	semaphore = (uv_sem_t*)malloc(sizeof(uv_sem_t));
	server_handle = (uv_stream_t*)malloc(sizeof(uv_stream_t));

	async_handle = (uv_async_t*)malloc(sizeof(uv_async_t));
	uv_barrier_wait(barrier);

	r = uv_async_init(loop, async_handle,
		[](uv_async_t* handle) {
			uv_close((uv_handle_t*)handle, NULL);
		});

	uv_unref((uv_handle_t*)async_handle);

	r = uv_sem_init(semaphore, 0);

	ipc_pipe->data = this;
    ready = true;
	wait();
	get_handle(loop);
	post();

	r = uv_run(loop, UV_RUN_DEFAULT);
}

void tcp_listener::on_ipc_connect(uv_connect_t* req, int status){
	int r;

	r = uv_read_start((uv_stream_t*)req->handle,
		[](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
		tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(handle->data);
		tcp_listener_ptr->on_ipc_alloc(handle, suggested_size, buf);
	},
		[](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf){
		tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(client->data);
		tcp_listener_ptr->on_ipc_read(client, nread, buf);
	});

	if (r) UV_ERR_TCP("ipc_handle_dispatcher read error", r);

}
void tcp_listener::on_ipc_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf){
	int r;

    uv_loop_t* loop;
    uv_handle_type type;
    uv_pipe_t* ipc_pipe;

    ipc_pipe = (uv_pipe_t*)client;
    loop = ipc_pipe->loop;

    r = uv_pipe_pending_count(ipc_pipe);
    type = uv_pipe_pending_type(ipc_pipe);


    uv_tcp_init(loop, (uv_tcp_t*)server_handle);
	r = uv_accept(client, server_handle);
    if (r) UV_ERR_TCP("uv_accept error", r);

    uv_close((uv_handle_t*)ipc_pipe, NULL);

	server_handle->data = this;

	r = uv_listen((uv_stream_t*)server_handle, 128,
		[](uv_stream_t* server_handle, int status) {
			tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(server_handle->data);
			tcp_listener_ptr->on_tcp_connect(server_handle, status);
		});
	if (r) UV_ERR_TCP("tcp listen error", r);


	r = uv_run(loop, UV_RUN_DEFAULT);

}
void tcp_listener::on_ipc_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
	buf->base = scratch;
	buf->len = sizeof(scratch);
}

void tcp_listener::on_tcp_connect(uv_stream_t* handle, int status){

	int r;
	client_t* client = (client_t*)malloc(sizeof(client_t));
	client->data = server_handle->data;

	r = uv_tcp_init(server_handle->loop, &client->stream);
	r = uv_accept(server_handle, (uv_stream_t*)&client->stream);
	r = uv_read_start((uv_stream_t*)&client->stream,
		[](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
			*buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
		},
		[](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
			tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(handle->data);
			tcp_listener_ptr->on_tcp_read(handle, nread, buf);
		});

}

void tcp_listener::on_tcp_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
	client_t *client = (client_t *)handle->data;
}

