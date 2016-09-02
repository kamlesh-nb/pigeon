//
// Created by root on 9/1/16.
//

#ifndef PIGEON_IPC_H
#define PIGEON_IPC_H


#include <uv.h>
#include <string>
#include <vector>

#include "tcp_listener.h"

class ipc_handle_dispatcher
{

private:
    uv_loop_t *loop;
    uv_pipe_t* ipc_server;
    uv_tcp_t* tcp_server;

    uv_async_t* async_handle;
    uv_barrier_t* barrier;

    struct sockaddr_in listen_addr;
    std::string pipe_name;
    std::vector<tcp_listener*> listeners;
    void init_tcp();
    void init_ipc();

public:
    ipc_handle_dispatcher();
    ~ipc_handle_dispatcher();

    void start();
    void on_ipc_connect(uv_stream_t *server, int status);
    void on_write(uv_write_t *req, int status);
};

#endif //PIGEON_IPC_H
