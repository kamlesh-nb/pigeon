#ifndef  TCP_LISTENER
#define TCP_LISTENER

#include <net/HttpServer.h>
#include "HttpServer.h"
#include "Settings.h"
#include "Listener.ipp"
#include "MessageParser.ipp"

using namespace pigeon::net;

#ifdef _WIN32
#define PIPENAME "\\\\?\\pipe\\"
#elif
#define PIPENAME "/tmp/"
#endif


class HttpServer::TcpListener : public HttpServer::Listener {

private:

    void get_handle(uv_loop_t* loop){
        uv_pipe_init(loop, ipc_pipe, 1);
        uv_pipe_connect(conn_req,
                        ipc_pipe,
                        pipe_name.c_str(),
                        [](uv_connect_t* req, int status){
                            TcpListener* tcp_listener_ptr = static_cast<TcpListener*>(req->data);
                            tcp_listener_ptr->OnIpcConnect(req, status);
                        });

    }

public:

    TcpListener(FileCache* p_cache){
        msgParser = new MessageParser;
        requestHandler = new RequestHandler(p_cache);

    }
    virtual ~TcpListener(){}

    virtual void Create() override {
        pipe_name.append(PIPENAME);
        pipe_name.append(Settings::ServiceName);
        t = std::thread(&TcpListener::Start, this);
    }

    virtual void Start() override {
        int r;

        loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
        ipc_pipe = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
        conn_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
        semaphore = (uv_sem_t*)malloc(sizeof(uv_sem_t));
        server_handle = (uv_stream_t*)malloc(sizeof(uv_stream_t));

        uv_loop_init(loop);
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

        r = uv_read_start(uv_connect->handle,
                          [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
                              TcpListener* tcp_listener_ptr = static_cast<TcpListener*>(handle->data);
                              tcp_listener_ptr->OnIpcAlloc(handle, suggested_size, buf);
                          },
                          [](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf){
                              TcpListener* tcp_listener_ptr = static_cast<TcpListener*>(client->data);
                              tcp_listener_ptr->OnIpcRead(client, nread, buf);
                          });

        if (r) LOG_UV_ERR("uv_read_start ipc read error", r);

    }

    virtual void OnIpcRead(uv_stream_t *stream, ssize_t ssize, const uv_buf_t *buf) override {
        int r;

        uv_loop_t* loop;
        uv_pipe_t* ipc_pipe;

        ipc_pipe = (uv_pipe_t*)stream;
        loop = ipc_pipe->loop;

        uv_tcp_init(loop, (uv_tcp_t*)server_handle);
        r = uv_accept(stream, server_handle);
        if (r) LOG_UV_ERR("uv_accept error", r);

        uv_close((uv_handle_t*)ipc_pipe, NULL);

        server_handle->data = this;

        r = uv_listen((uv_stream_t*)server_handle, 128,
                      [](uv_stream_t* server_handle, int status) {
                          TcpListener* tcp_listener_ptr = static_cast<TcpListener*>(server_handle->data);
                          tcp_listener_ptr->OnTcpConnect(server_handle, status);
                      });
        if (r) LOG_UV_ERR("tcp listen error", r);

        free(conn_req);

        r = uv_run(loop, UV_RUN_DEFAULT);
        if (r) LOG_UV_ERR("uv_run error", r);
    }

    virtual void OnIpcAlloc(uv_handle_t *handle, size_t size, uv_buf_t *buf) override {
        buf->base = scratch;
        buf->len = sizeof(scratch);
    }

    virtual void OnTcpConnect(uv_stream_t *stream, int i) override {
        int r;
        client_t* client = (client_t*)malloc(sizeof(client_t));
        msgParser->Init(client->parser, HTTP_REQUEST);
        server_handle->data = this;
        client->context = new HttpContext;
        client->parser.data = client;
        client->stream.data = client;
        client->data = this;
        client->context->data = client;
        client->requestHandler = requestHandler;
        client->loop = loop;

        r = uv_tcp_init(server_handle->loop, &client->stream);
        r = uv_accept(server_handle, (uv_stream_t*)&client->stream);
        client->client_address = GetClientAddress(&client->stream);
        r = uv_read_start((uv_stream_t*)&client->stream,
                          [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                              *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                          },
                          [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                              TcpListener* tcp_listener_ptr = static_cast<TcpListener*>(((client_t*)handle->data)->data);
                              tcp_listener_ptr->OnTcpRead(handle, nread, buf);
                          });

    }

    virtual void OnTcpRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) override {
        ssize_t parsed;
        client_t *client = (client_t *)stream->data;
        if (nread >= 0) {
            parsed = msgParser->Parse(client, buf->base, nread);
            if (parsed < nread) {
                //use Logger to log error
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
                //use Logger to log error
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

    }

    void AddRoute(std::string route, int method, OnHandlerExecution func) override {
        requestHandler->AddRoute(route, method, func);
    }

    void AddFilter(std::string filter, OnHandlerExecution func) override {
        requestHandler->AddFilter(filter, func);
    }


};

#endif //TCP_LISTENER