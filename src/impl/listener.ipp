#ifndef LISTENER
#define LISTENER

#include <thread>
#include "http_server.h"

using  namespace pigeon;

class http_server::listener {

protected:

    uv_sem_t* semaphore;
    uv_loop_t* loop;
    uv_tcp_t* tcp_server;
    uv_pipe_t* ipc_pipe;
    uv_connect_t* conn_req;
    uv_stream_t* server_handle;
    uv_async_t* async_handle;
    msg_parser* msgParser;
    request_processor* requestProcessor;

    char scratch[16];

    std::thread t;
    std::string pipe_name;

public:

    virtual ~listener(){}
    bool ready = false;
    virtual void create(){};
    virtual void start(){};

    virtual void post(){};
    virtual void wait(){};

    virtual void on_ipc_connect(uv_connect_t*, int){};
    virtual void on_ipc_read(uv_stream_t*, ssize_t, const uv_buf_t*){};
    virtual void on_ipc_alloc(uv_handle_t*, size_t, uv_buf_t*){};
    virtual void on_tcp_connect(uv_stream_t*, int){};
    virtual void on_tcp_read(uv_stream_t*, ssize_t, const uv_buf_t*){};
    virtual void on_tcp_write(uv_write_t*, int){};

};

#endif //LISTENER