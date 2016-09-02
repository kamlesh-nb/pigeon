//
// Created by root on 9/1/16.
//

#include "ipc_handle_dispatcher.h"
#include <assert.h>


#define UV_ERR_IPC(msg, code) do {                                                         \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);

ipc_handle_dispatcher::ipc_handle_dispatcher()
{
}

ipc_handle_dispatcher::~ipc_handle_dispatcher()
{
}

void ipc_handle_dispatcher::init_ipc(){
    int r;

    pipe_name = "/tmp/uv";

    uv_fs_t req;
    uv_fs_unlink(loop, &req, "/tmp/uv", NULL);

    ipc_server->data = this;
    uv_pipe_init(loop, ipc_server, 1);

    r = uv_pipe_bind(ipc_server, pipe_name.c_str());
    if (r) UV_ERR_IPC("ipc_handle_dispatcher bind error", r);

    r = uv_listen((uv_stream_t*)ipc_server, 128,
                  [](uv_stream_t *server, int status){
                      ipc_handle_dispatcher* ipc_ptr = static_cast<ipc_handle_dispatcher*>(server->data);
                      ipc_ptr->on_ipc_connect(server, status);
                  });

    if (r) UV_ERR_IPC("ipc_handle_dispatcher listen error", r);

}
void ipc_handle_dispatcher::init_tcp(){
    int r;
    r = uv_ip4_addr("127.0.0.1", 8088, &listen_addr);
    r = uv_tcp_init(loop, (uv_tcp_t*)tcp_server);
    r = uv_tcp_nodelay((uv_tcp_t*)tcp_server, 1);
    r = uv_tcp_bind((uv_tcp_t*)tcp_server, (const struct sockaddr*) &listen_addr, 0);

}

void ipc_handle_dispatcher::start(){

    loop = uv_default_loop();

    ipc_server = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
    tcp_server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

    barrier = (uv_barrier_t*)malloc(sizeof(uv_barrier_t));
    async_handle = (uv_async_t*)malloc(sizeof(uv_async_t));

    uv_barrier_init(barrier, 1 + 1);
    uv_async_init(loop, async_handle, NULL);


    for (int i = 0; i <= 3; ++i) {
        listeners.push_back(new tcp_listener);
        listeners[i]->create(barrier);
    }

    uv_barrier_wait(barrier);
    init_tcp();
    init_ipc();

    for (auto& l : listeners) {
        if(l->ready)
            l->post();
    }

    uv_run(loop, UV_RUN_DEFAULT);

}

void ipc_handle_dispatcher::on_ipc_connect(uv_stream_t *server, int status){
    int r;
    uv_buf_t buf;
    uv_write_t write_req;
    uv_loop_t *cloop;

    cloop = server->loop;

    const char* ping = "PING";
    buf = uv_buf_init((char*)ping, 4);

    uv_pipe_t *client = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
    r = uv_pipe_init(cloop, client, 1);
    r = uv_accept(server, (uv_stream_t*)client);
    write_req.data = this;

    r = uv_write2(&write_req,
                  (uv_stream_t*)client,
                  &buf,
                  1,
                  (uv_stream_t*)tcp_server,
                  [](uv_write_t* req, int status) {
                      ipc_handle_dispatcher* ipc_ptr = static_cast<ipc_handle_dispatcher*>(req->data);
                      ipc_ptr->on_write(req, status);
                  });

    if (r) UV_ERR_IPC("ipc_handle_dispatcher write error", r);


}

void ipc_handle_dispatcher::on_write(uv_write_t *req, int status){
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
   // free(req);
}