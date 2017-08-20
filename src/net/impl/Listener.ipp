#ifndef LISTENER
#define LISTENER

#include <thread>
#include "HttpServer.h"
#include "RequestHandler.h"


using  namespace pigeon::net;

class HttpServer::Listener {
private:
    char buf[32];
protected:
    uv_sem_t* semaphore;
    uv_loop_t* loop;
    uv_tcp_t* tcp_server;
    uv_pipe_t* ipc_pipe;
    uv_connect_t* conn_req;
    uv_stream_t* server_handle;
    MessageParser* msgParser;
    RequestHandler* requestHandler;

    char scratch[16];

    std::thread t;
    std::string pipe_name;

    char* GetClientAddress(const uv_tcp_t *handle){

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

    virtual ~Listener(){}
    bool ready = false;
    virtual void Create(){};
    virtual void Start(){};

    virtual void Post(){};
    virtual void Wait(){};

    virtual void OnIpcConnect(uv_connect_t*, int){};
    virtual void OnIpcRead(uv_stream_t *, ssize_t, const uv_buf_t *){};
    virtual void OnIpcAlloc(uv_handle_t *, size_t, uv_buf_t *){};
    virtual void OnTcpConnect(uv_stream_t *, int){};
    virtual void OnTcpRead(uv_stream_t *, ssize_t, const uv_buf_t *){};
    virtual void AddRoute(std::string route, int method, OnHandlerExecution func){
    }
    virtual void AddFilter(std::string filter, OnHandlerExecution func){
    }

};

#endif //LISTENER