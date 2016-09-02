//
// Created by root on 9/1/16.
//

#ifndef PIGEON_WORKER_H
#define PIGEON_WORKER_H



#include <uv.h>
#include <thread>
#include <string>

class tcp_listener
{

private:
    uv_sem_t* semaphore;
    uv_loop_t* loop;
    uv_tcp_t* tcp_server;
    uv_pipe_t* ipc_pipe;
    uv_connect_t* conn_req;
    uv_stream_t* server_handle;

    uv_async_t* async_handle;
    uv_barrier_t* barrier;


    char scratch[16];

    std::thread t;
    std::string pipe_name;

    void get_handle(uv_loop_t*);

public:
    tcp_listener();
    ~tcp_listener();

    bool ready = false;
    void create(uv_barrier_t*);
    void start();

    void post();
    void wait();

    void on_ipc_connect(uv_connect_t* req, int status);
    void on_ipc_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
    void on_ipc_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    void on_tcp_connect(uv_stream_t*, int);
    void on_tcp_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);
};
#endif //PIGEON_WORKER_H
