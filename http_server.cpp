//
// Created by kamlesh on 30/10/15.
//

#include <uv.h>
#include <malloc.h>
#include "http_server.h"
#include <logger.h>





#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

namespace pigeon {

    string IPC_PIPE_NAME;

    settings* _Settings;
    cache* _Cache;

    http_parser_settings parser_settings;

    typedef struct {
        uv_tcp_t handle;
        uv_shutdown_t shutdown_req;
    } conn_rec_t;

    typedef struct {

        uv_tcp_t stream;
        http_parser parser;
        uv_write_t write_req;
        char* client_adress;
        http_context* context;

    } iconnection_t;

    typedef struct {

        uv_work_t request;
        iconnection_t* iConn;
        bool error;
        char* result;
        size_t length;

    } msg_baton_t;

    union stream_handle {
        uv_pipe_t pipe;
        uv_tcp_t tcp;
    };

    typedef unsigned char handle_storage_t[sizeof(union stream_handle)];


    struct ipc_server_ctx {
        handle_storage_t server_handle;
        unsigned int num_connects;
        uv_pipe_t ipc_pipe;
    };

    struct ipc_peer_ctx {
        handle_storage_t peer_handle;
        uv_write_t write_req;
    };

    struct ipc_client_ctx {
        uv_connect_t connect_req;
        uv_stream_t* server_handle;
        uv_pipe_t ipc_pipe;
        char scratch[16];
    };

    struct server_ctx {
        uv_loop_t* loop;
        handle_storage_t server_handle;
        unsigned int num_connects;
        uv_async_t async_handle;
        uv_thread_t thread_id;
        uv_sem_t semaphore;
    };

    void ipc_connection_cb(uv_stream_t* ipc_pipe, int status);
    void ipc_write_cb(uv_write_t* req, int status);
    void ipc_close_cb(uv_handle_t* handle);
    void ipc_connect_cb(uv_connect_t* req, int status);
    void ipc_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);
    void ipc_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

    void sv_async_cb(uv_async_t* handle);
    void sv_connection_cb(uv_stream_t* server_handle, int status);
    void sv_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);
    void sv_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

    struct sockaddr_in listen_addr;
    uv_async_t* listener_async_handles;
    uv_barrier_t* listeners_created_barrier;
    uv_tcp_t server;

    void ipc_connection_cb(uv_stream_t* ipc_pipe, int status) {
        int r;
        struct ipc_server_ctx* sc;
        struct ipc_peer_ctx* pc;
        uv_loop_t* loop;
        uv_buf_t buf;

        loop = ipc_pipe->loop;
        buf = uv_buf_init("PING", 4);
        sc = container_of(ipc_pipe, struct ipc_server_ctx, ipc_pipe);
        pc = (struct ipc_peer_ctx*)calloc(1, sizeof(*pc));


        if (ipc_pipe->type == UV_TCP)
            r = uv_tcp_init(loop, (uv_tcp_t*) &pc->peer_handle);
        else if (ipc_pipe->type == UV_NAMED_PIPE)
            r = uv_pipe_init(loop, (uv_pipe_t*) &pc->peer_handle, 1);

        r = uv_accept(ipc_pipe, (uv_stream_t*) &pc->peer_handle);
        r = uv_write2(&pc->write_req, (uv_stream_t*) &pc->peer_handle, &buf, 1, (uv_stream_t*) &sc->server_handle, ipc_write_cb);

        if (--sc->num_connects == 0)
            uv_close((uv_handle_t*) ipc_pipe, NULL);

    }

    void ipc_write_cb(uv_write_t* req, int status) {
        struct ipc_peer_ctx* ctx;
        ctx = container_of(req, struct ipc_peer_ctx, write_req);
        uv_close((uv_handle_t*) &ctx->peer_handle, ipc_close_cb);
    }

    void ipc_close_cb(uv_handle_t* handle) {
        struct ipc_peer_ctx* ctx;
        ctx = container_of(handle, struct ipc_peer_ctx, peer_handle);
        free(ctx);
    }

    void ipc_connect_cb(uv_connect_t* req, int status) {
        int r;
        struct ipc_client_ctx* ctx;
        ctx = container_of(req, struct ipc_client_ctx, connect_req);
        r = uv_read_start((uv_stream_t*) &ctx->ipc_pipe, ipc_alloc_cb, ipc_read_cb);

    }

    void ipc_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        struct ipc_client_ctx* ctx;
        ctx = container_of(handle, struct ipc_client_ctx, ipc_pipe);
        buf->base = ctx->scratch;
        buf->len = sizeof(ctx->scratch);
    }

    void ipc_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
        int r;
        struct ipc_client_ctx* ctx;
        uv_loop_t* loop;
        uv_handle_type type;
        uv_pipe_t* ipc_pipe;

        ipc_pipe = (uv_pipe_t*) handle;
        ctx = container_of(ipc_pipe, struct ipc_client_ctx, ipc_pipe);
        loop = ipc_pipe->loop;

        r = uv_pipe_pending_count(ipc_pipe);
        type = uv_pipe_pending_type(ipc_pipe);

        if (type == UV_TCP)
            r = uv_tcp_init(loop, (uv_tcp_t*) ctx->server_handle);
        else if (type == UV_NAMED_PIPE)
            r = uv_pipe_init(loop, (uv_pipe_t*) ctx->server_handle, 0);

        r = uv_accept(handle, ctx->server_handle);
        uv_close((uv_handle_t*) &ctx->ipc_pipe, NULL);
    }

    void send_listen_handles(uv_handle_type type, unsigned int num_servers, struct server_ctx* servers) {
        int r;
        struct ipc_server_ctx ctx;
        uv_loop_t* loop;
        unsigned int i;

        loop = uv_default_loop();
        ctx.num_connects = num_servers;

        if (type == UV_TCP) {
            r = uv_tcp_init(loop, (uv_tcp_t*) &ctx.server_handle);
            r = uv_tcp_bind((uv_tcp_t*) &ctx.server_handle, (const struct sockaddr*) &listen_addr, 0);
        }

        r = uv_pipe_init(loop, &ctx.ipc_pipe, 1);
        r = uv_pipe_bind(&ctx.ipc_pipe, IPC_PIPE_NAME.c_str());
        r = uv_listen((uv_stream_t*) &ctx.ipc_pipe, 128, ipc_connection_cb);

        for (i = 0; i < num_servers; i++)
            uv_sem_post(&servers[i].semaphore);

        r = uv_run(loop, UV_RUN_DEFAULT);
        uv_close((uv_handle_t*) &ctx.server_handle, NULL);
        r = uv_run(loop, UV_RUN_DEFAULT);

        for (i = 0; i < num_servers; i++)
            uv_sem_wait(&servers[i].semaphore);

    }

    void get_listen_handle(uv_loop_t* loop, uv_stream_t* server_handle) {
        int r;
        struct ipc_client_ctx ctx;
        ctx.server_handle = server_handle;
        ctx.server_handle->data = (void*)"server handle";

        r = uv_pipe_init(loop, &ctx.ipc_pipe, 1);
        uv_pipe_connect(&ctx.connect_req, &ctx.ipc_pipe, IPC_PIPE_NAME.c_str(), ipc_connect_cb);
        r = uv_run(loop, UV_RUN_DEFAULT);

    }

    void server_cb(void *arg) {

        int r;
        struct server_ctx *ctx;
        uv_loop_t* loop;

        ctx = (struct server_ctx *)arg;
        loop = uv_loop_new();

        ctx->loop = loop;

        uv_barrier_wait(listeners_created_barrier);

        r = uv_async_init(loop, &ctx->async_handle, sv_async_cb);
        uv_unref((uv_handle_t*) &ctx->async_handle);

        uv_sem_wait(&ctx->semaphore);
        get_listen_handle(loop, (uv_stream_t*) &ctx->server_handle);
        uv_sem_post(&ctx->semaphore);

        r = uv_listen((uv_stream_t*) &ctx->server_handle, 128, sv_connection_cb);
        r = uv_run(loop, UV_RUN_DEFAULT);

        uv_loop_delete(loop);

    }

    void sv_async_cb(uv_async_t* handle) {
        struct server_ctx* ctx;
        ctx = container_of(handle, struct server_ctx, async_handle);
        uv_close((uv_handle_t*) &ctx->server_handle, NULL);
        uv_close((uv_handle_t*) &ctx->async_handle, NULL);
    }

    void sv_connection_cb(uv_stream_t* server_handle, int status) {

        int r;
        iconnection_t* iConn = (iconnection_t*)malloc(sizeof(iconnection_t));
        iConn->context = new http_context;
        iConn->context->Settings = _Settings;
        iConn->context->Cache = _Cache;

        iConn->parser.data = iConn;
        iConn->stream.data = iConn;

        r = uv_tcp_init(server_handle->loop, &iConn->stream);
        http_parser_init(&iConn->parser, HTTP_REQUEST);

        r = uv_accept(server_handle, (uv_stream_t*) &iConn->stream);
        r = uv_read_start((uv_stream_t*) &iConn->stream, sv_alloc_cb, sv_read_cb);

    }

    void sv_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        buf->base = (char*)malloc(suggested_size);
        buf->len = suggested_size;
    }

    auto on_close(uv_handle_t *handle) -> void {

        try {

            iconnection_t* iConn = (iconnection_t*)handle->data;
            delete iConn->context;
            free(iConn);

        }
        catch (std::exception ex) {

        }
    }

    auto on_shutdown(uv_shutdown_t* req, int status) -> void {

        conn_rec_t* conn = container_of(req, conn_rec_t, shutdown_req);
        msg_baton_t *closure = static_cast<msg_baton_t *>(req->data);
        delete closure;
        uv_close((uv_handle_t*)&conn->handle, on_close);

    }


    void sv_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
        try {

            ssize_t parsed;
            iconnection_t *iConn = (iconnection_t *)handle->data;
            if (nread >= 0) {
                parsed = (ssize_t)http_parser_execute(&iConn->parser, &parser_settings, buf->base, nread);
                if (parsed < nread) {
                    logger::get(_Settings)->write(LogType::Error, Severity::Critical, "parse failed");
                    uv_close((uv_handle_t *)&iConn->stream, on_close);
                }
            }
            else {
                if (nread != UV_EOF) {
                    logger::get(_Settings)->write(LogType::Error, Severity::Critical, "read failed");
                }
                uv_close((uv_handle_t *)&iConn->stream, on_close);
            }
            free(buf->base);
        }
        catch (std::exception& ex){

            throw std::runtime_error(ex.what());

        }
    }


    void http_server::init_server() {

        _Settings = new settings;
        _Settings->load_setting();
        _Cache = new cache;
        _Cache->load(_Settings->get_resource_location());

#ifdef _WIN32
        IPC_PIPE_NAME += "\\\\?\\pipe\\";
        IPC_PIPE_NAME += _Settings->get_service_name();
#else
        IPC_PIPE_NAME +=  "/tmp/";
        IPC_PIPE_NAME += _Settings->get_service_name();
#endif

    }

    void http_server::start() {

        init_server();
        init_parser();

        uv_async_t* service_handle = 0;

        struct server_ctx* servers;
        struct client_ctx* clients;
        uv_loop_t* loop;

        unsigned int i;
        double time;
        int r;

        int port = _Settings->get_port();
        string addr = _Settings->get_address();
        unsigned int no_of_threads = _Settings->get_num_worker_threads();
        r = uv_ip4_addr(addr.c_str(), port, &listen_addr);

        loop = uv_default_loop();
        uv_tcp_init(loop, &server);

        listener_async_handles = (uv_async_t*)calloc(no_of_threads, sizeof(uv_async_t));

        listeners_created_barrier = (uv_barrier_t*)malloc(sizeof(uv_barrier_t));
        uv_barrier_init(listeners_created_barrier, no_of_threads + 1);

        service_handle = (uv_async_t*)malloc(sizeof(uv_async_t));
        uv_async_init(loop, service_handle, NULL);


        servers = (struct server_ctx*)calloc(no_of_threads, sizeof(servers[0]));

        for (i = 0; i < no_of_threads; i++) {
            struct server_ctx* ctx = servers + i;
            r = uv_sem_init(&ctx->semaphore, 0);
            r = uv_thread_create(&ctx->thread_id, server_cb, ctx);
        }
        uv_barrier_wait(listeners_created_barrier);

        send_listen_handles(UV_TCP, no_of_threads, servers);
        uv_run(loop, UV_RUN_DEFAULT);

    }


    auto on_send_complete(uv_write_t *req, int status) -> void {

        try {

            if (status != 0){
                logger::get(_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(status));
            }

            conn_rec_t* conn = container_of(req, conn_rec_t, shutdown_req);

            size_t _write_queue_size = ((uv_stream_t *)conn)->write_queue_size;

            if (uv_is_writable((uv_stream_t *)req) && _write_queue_size > 0) {
                uv_shutdown(&conn->shutdown_req, (uv_stream_t *)conn, on_shutdown);
            }
            else {
                if (!uv_is_closing((uv_handle_t*)req->handle))
                {
                    uv_close((uv_handle_t*)req->handle, on_close);
                }
            }

        }
        catch (std::exception& ex){
            throw std::runtime_error(ex.what());
        }

    }


    auto on_url(http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            iConn->context->request->url += data;
            free(data);
        }
        return 0;

    }

    auto on_header_field(http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);

            iConn->context->request->set_header_field(s);
        }
        return 0;

    }

    auto on_header_value(http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);
            iConn->context->request->set_header_value(s);
        }
        return 0;

    }

    auto on_headers_complete(http_parser* parser) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        iConn->context->request->method = parser->method;
        iConn->context->request->http_major_version = parser->http_major;
        iConn->context->request->http_minor_version = parser->http_minor;
        return 0;

    }

    auto on_body(http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {

            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            iConn->context->request->content += data;
            free(data);

        }
        return 0;

    }

    auto on_message_complete(http_parser* parser) -> int {


        iconnection_t* iConn = (iconnection_t*)parser->data;


        iConn->context->request->is_api = is_api(iConn->context->request->url);
        parse_query_string(*iConn->context->request);

       // process(iConn->context);
        std::string msg = "<html><body><center><h1>Hello World!</h1></center></body></html>";
        std::string s = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: keep-alive\r\nContent-Length: ";

        s +=  std::to_string(msg.size());
        s += "\r\n";
        s += msg;



        uv_buf_t resbuf;
        resbuf.base = (char*)s.c_str();
        resbuf.len =  s.size();



        int r = uv_write(&iConn->write_req,
                         (uv_stream_t*)&iConn->stream,
                         &resbuf,
                         1,
                         on_send_complete);


        return 0;

    }



    void http_server::init_parser() {
       parser_settings.on_url = on_url;
       parser_settings.on_header_field = on_header_field;
       parser_settings.on_header_value = on_header_value;
       parser_settings.on_headers_complete = on_headers_complete;
       parser_settings.on_body = on_body;
       parser_settings.on_message_complete = on_message_complete;
    }
}