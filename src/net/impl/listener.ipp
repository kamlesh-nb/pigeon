#ifndef LISTENER
#define LISTENER

#include <thread>
#include "http_server.h"
#include "request_handler.h"


using  namespace pigeon::net;

class http_server::listener {
private:
    char buf[32];
protected:
    uv_sem_t* semaphore;
    uv_loop_t* loop;
    uv_tcp_t* tcp_server;
    uv_pipe_t* ipc_pipe;
    uv_connect_t* conn_req;
    uv_stream_t* server_handle;
    msg_parser* msgParser;
    request_handler* requestHandler;
    db_connection* dbConnection;

    char scratch[16];

    std::thread t;
    std::string pipe_name;

    char* get_client_address(const uv_tcp_t* handle){

        struct sockaddr_in name;
        int namelen = sizeof(name);
        int r = uv_tcp_getpeername(handle, (struct sockaddr*) &name, &namelen);
        if (r) LOG_UV_ERR("uv_tcp_getpeername error", r);
        char addr[16];
        uv_inet_ntop(AF_INET, &name.sin_addr, addr, sizeof(addr));
        snprintf(buf, sizeof(buf), "%s:%d", addr, ntohs(name.sin_port));
        return buf;
    }

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
    virtual void add_route(std::string route, int method, request_process_step_cb func){
    }
    virtual void add_filter(std::string filter, request_process_step_cb func){
    }

};

#endif //LISTENER