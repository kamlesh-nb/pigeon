#ifndef  TCP_LISTENER
#define TCP_LISTENER

#include <http_server.h>
#include "listener.ipp"
#include "msg_parser.ipp"

using namespace pigeon;

class http_server::tcp_listener : public http_server::listener {

private:

    void get_handle(uv_loop_t*){
        uv_pipe_init(loop, ipc_pipe, 1);
        uv_pipe_connect(conn_req,
                        ipc_pipe,
                        pipe_name.c_str(),
                        [](uv_connect_t* req, int status){
                            tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(req->data);
                            tcp_listener_ptr->on_ipc_connect(req, status);
                        });

    }

public:

    tcp_listener(){ msgParser = new msg_parser; requestProcessor = new request_processor; }
    virtual ~tcp_listener(){}

    virtual void create() override {
        pipe_name = "/tmp/";
        pipe_name.append(app_context::get()->get_service_name());
        t = std::thread(&tcp_listener::start, this);
    }

    virtual void start() override {
        int r;

        loop = uv_loop_new();

        ipc_pipe = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
        conn_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
        semaphore = (uv_sem_t*)malloc(sizeof(uv_sem_t));
        server_handle = (uv_stream_t*)malloc(sizeof(uv_stream_t));

        async_handle = (uv_async_t*)malloc(sizeof(uv_async_t));

        r = uv_async_init(loop, async_handle,
                          [](uv_async_t* handle) {
                              uv_close((uv_handle_t*)handle, NULL);
                          });

        uv_unref((uv_handle_t*)async_handle);

        r = uv_sem_init(semaphore, 0);
        conn_req->data = this;
        ipc_pipe->data = this;
        ready = true;
        wait();
        get_handle(loop);
        post();

        r = uv_run(loop, UV_RUN_DEFAULT);
    }

    virtual void post() override {
        uv_sem_post(semaphore);
    }

    virtual void wait() override {
        uv_sem_wait(semaphore);
    }

    virtual void on_ipc_connect(uv_connect_t *uv_connect, int i) override {
        int r;

        r = uv_read_start((uv_stream_t*)uv_connect->handle,
                          [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
                              tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(handle->data);
                              tcp_listener_ptr->on_ipc_alloc(handle, suggested_size, buf);
                          },
                          [](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf){
                              tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(client->data);
                              tcp_listener_ptr->on_ipc_read(client, nread, buf);
                          });

        if (r) LOG_UV_ERR("tcl_listener ipc read error", r);
    }

    virtual void on_ipc_read(uv_stream_t *stream, ssize_t ssize, const uv_buf_t *buf) override {
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
                          tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(server_handle->data);
                          tcp_listener_ptr->on_tcp_connect(server_handle, status);
                      });
        if (r) LOG_UV_ERR("tcp listen error", r);


        r = uv_run(loop, UV_RUN_DEFAULT);
    }

    virtual void on_ipc_alloc(uv_handle_t *handle, size_t size, uv_buf_t *buf) override {
        buf->base = scratch;
        buf->len = sizeof(scratch);
    }

    virtual void on_tcp_connect(uv_stream_t *stream, int i) override {
        int r;
        client_t* client = (client_t*)malloc(sizeof(client_t));
        msgParser->init(client->parser, HTTP_REQUEST);
        server_handle->data = this;
        client->data = server_handle->data;
        client->context = new http_context;
        client->parser.data = client;
        client->stream.data = client;
        client->data = server_handle->data;
        client->reqproc_ptr = requestProcessor;
        client->context->data = stream->loop;

        r = uv_tcp_init(server_handle->loop, &client->stream);
        r = uv_accept(server_handle, (uv_stream_t*)&client->stream);
        r = uv_read_start((uv_stream_t*)&client->stream,
                          [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                              *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                          },
                          [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                              tcp_listener* tcp_listener_ptr = static_cast<tcp_listener*>(((client_t*)handle->data)->data);
                              tcp_listener_ptr->on_tcp_read(handle, nread, buf);
                          });

    }

    virtual void on_tcp_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) override {
        ssize_t parsed;
        client_t *client = (client_t *)stream->data;
        if (nread >= 0) {
            parsed = msgParser->parse(client, buf->base, nread);
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

    }


};

#endif //TCP_LISTENER