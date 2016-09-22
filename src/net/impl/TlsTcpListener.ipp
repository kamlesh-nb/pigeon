#ifndef TLS_TCP_LISTENER
#define TLS_TCP_LISTENER

#include <net/HttpServer.h>
#include "Listener.ipp"
#include "MessageParser.ipp"

using  namespace pigeon;


class HttpServer::TlsTcpListener : public HttpServer::Listener {

private:
    void get_handle(uv_loop_t*){
        uv_pipe_init(loop, ipc_pipe, 1);
        uv_pipe_connect(conn_req,
                        ipc_pipe,
                        pipe_name.c_str(),
                        [](uv_connect_t* req, int status){
                            TlsTcpListener* tls_tcp_listener_ptr = static_cast<TlsTcpListener*>(req->data);
                            tls_tcp_listener_ptr->OnIpcConnect(req, status);
                        });
    }

public:

    TlsTcpListener(FileCache* p_cache){ msgParser = new MessageParser; requestHandler = new RequestHandler(p_cache); }
    virtual ~TlsTcpListener(){}

    virtual void Create() override {
        pipe_name = "/tmp/";
        pipe_name.append(Settings::ServiceName);
        t = std::thread(&TlsTcpListener::Start, this);
    }

    virtual void Start() override {
        int r;

        loop = uv_loop_new();

        ipc_pipe = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
        conn_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
        semaphore = (uv_sem_t*)malloc(sizeof(uv_sem_t));
        server_handle = (uv_stream_t*)malloc(sizeof(uv_stream_t));

        r = uv_sem_init(semaphore, 0);
        conn_req->data = this;
        ipc_pipe->data = this;


        ready = true;
        Wait();
        get_handle(loop);
        Post();

        r = uv_run(loop, UV_RUN_DEFAULT);
    }

    virtual void Post() override {
        uv_sem_post(semaphore);
    }

    virtual void Wait() override {
        uv_sem_wait(semaphore);
    }

    virtual void OnIpcConnect(uv_connect_t *uv_connect, int i) override {
        int r;

        r = uv_read_start((uv_stream_t*)uv_connect->handle,
                          [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
                              TlsTcpListener* tls_tcp_listener_ptr = static_cast<TlsTcpListener*>(handle->data);
                              tls_tcp_listener_ptr->OnIpcAlloc(handle, suggested_size, buf);
                          },
                          [](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf){
                              TlsTcpListener* tls_tcp_listener_ptr = static_cast<TlsTcpListener*>(client->data);
                              tls_tcp_listener_ptr->OnIpcRead(client, nread, buf);
                          });

        if (r) LOG_UV_ERR("tcl_listener ipc read error", r);
    }

    virtual void OnIpcRead(uv_stream_t *stream, ssize_t ssize, const uv_buf_t *buf) override {
        int r;

        uv_loop_t* loop;
        uv_handle_type type;
        uv_pipe_t* ipc_pipe;

        ipc_pipe = (uv_pipe_t*)stream;
        loop = ipc_pipe->loop;

        r = uv_pipe_pending_count(ipc_pipe);
        type = uv_pipe_pending_type(ipc_pipe);


        uv_tcp_init(loop, (uv_tcp_t*)server_handle);
        r = uv_accept(stream, server_handle);
        if (r) LOG_UV_ERR("uv_accept error", r);

        uv_close((uv_handle_t*)ipc_pipe, NULL);

        server_handle->data = this;

        r = uv_listen((uv_stream_t*)server_handle, 128,
                      [](uv_stream_t* server_handle, int status) {
                          TlsTcpListener* tls_tcp_listener_ptr = static_cast<TlsTcpListener*>(server_handle->data);
                          tls_tcp_listener_ptr->OnTcpConnect(server_handle, status);
                      });
        if (r) LOG_UV_ERR("tcp listen error", r);

        r = uv_run(loop, UV_RUN_DEFAULT);
    }

    virtual void OnIpcAlloc(uv_handle_t *handle, size_t size, uv_buf_t *buf) override {
        buf->base = scratch;
        buf->len = sizeof(scratch);
    }

    virtual void OnTcpConnect(uv_stream_t *stream, int status) override {


    }

    virtual void OnTcpRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) override {


    }

    virtual void AddRoute(std::string route, int method, OnHandlerExecution func) override {

    }

    virtual void AddFilter(std::string filter, OnHandlerExecution func) override {

    }

};

#endif //TLS_TCP_LISTENER