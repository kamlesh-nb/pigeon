#include "settings.h"
#include "http_server.h"
#include "tcp_listener.ipp"
#include "tls_tcp_listener.ipp"



using namespace pigeon;


class http_server::handle_dispatcher {

private:

    uv_loop_t *loop;
    uv_pipe_t* ipc_server;
    uv_tcp_t* tcp_server;
    cache* m_cache;
    uint  threads;

    struct sockaddr_in listen_addr;
    std::string pipe_name;
    std::vector<listener*> listeners;

    void remove_fs_link(){
        pipe_name = "/tmp/";
        pipe_name.append(settings::service_name);

        uv_fs_t req;
        uv_fs_unlink(loop, &req, pipe_name.c_str(), NULL);
    }

    void init_tcp(){

        int r;

        string address = settings::address;
        int port = settings::port;

        r = uv_ip4_addr(address.c_str(), port, &listen_addr);

        r = uv_tcp_init(loop, (uv_tcp_t*)tcp_server);
        if (r) LOG_UV_ERR("handle_dispatcher tcp initialize error", r);

        r = uv_tcp_nodelay((uv_tcp_t*)tcp_server, 1);

        r = uv_tcp_bind((uv_tcp_t*)tcp_server, (const struct sockaddr*) &listen_addr, 0);
        if (r) LOG_UV_ERR("handle_dispatcher tcp bind error", r);

    }

    void init_ipc(){

        int r;

        remove_fs_link();
        ipc_server->data = this;
        uv_pipe_init(loop, ipc_server, 1);

        r = uv_pipe_bind(ipc_server, pipe_name.c_str());
        if (r) LOG_UV_ERR("handle_dispatcher pipe bind error", r);

        r = uv_listen((uv_stream_t*)ipc_server, 128,
                      [](uv_stream_t *server, int status){
                          handle_dispatcher* handle_dispatcher_ptr = static_cast<handle_dispatcher*>(server->data);
                          handle_dispatcher_ptr->on_ipc_connect(server, status);
                      });

        if (r) LOG_UV_ERR("handle_dispatcher pipe listen error", r);

    }




public:

    handle_dispatcher(cache* p_cache):m_cache(p_cache){}

    ~handle_dispatcher(){}
    void create_listeners(){

        int threads = settings::worker_threads;

        if(settings::use_ssl){
            for (int i = 0; i < threads; ++i) {
                listeners.push_back(new tls_tcp_listener(m_cache));
                //listeners[i]->create();
            }
        } else {
            for (int i = 0; i < threads; ++i) {
                listeners.push_back(new tcp_listener(m_cache));
                //listeners[i]->create();
            }
        }

    }


    void start(){

        loop = uv_default_loop();

        ipc_server = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
        tcp_server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

        for (auto& l : listeners) {
            l->create();
        }

        init_tcp();
        init_ipc();

        for (auto& l : listeners) {
            if(l->ready)
                l->post();
        }

        uv_run(loop, UV_RUN_DEFAULT);
        uv_close((uv_handle_t*)&ipc_server, NULL);

        uv_run(loop, UV_RUN_DEFAULT);

        for (auto& l : listeners) {
            if(l->ready)
                l->wait();
        }
    }

    void add_route(std::string route, int method, request_process_step_cb func){
        for(auto& l:listeners){
            l->add_route(route, method, func);
        }
    }

    void add_filter(std::string filter, request_process_step_cb func){
        for(auto& l:listeners){
            l->add_filter(filter, func);
        }
    }

    void on_ipc_connect(uv_stream_t *server, int status){
        int r;
        uv_buf_t buf;
        uv_write_t* write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
        uv_loop_t *cloop;

        cloop = server->loop;

        const char* ping = "PING";
        buf = uv_buf_init((char*)ping, 4);

        uv_pipe_t *client = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
        r = uv_pipe_init(cloop, client, 1);
        r = uv_accept(server, (uv_stream_t*)client);
        write_req->data = this;

        r = uv_write2(write_req,
                      (uv_stream_t*)client,
                      &buf,
                      1,
                      (uv_stream_t*)tcp_server,
                      [](uv_write_t* req, int status) {

                          if (status) LOG_UV_ERR("handle_dispatcher pipe on_write error", status);
                          if (!uv_is_closing((uv_handle_t *) req->handle)) {
                              uv_close((uv_handle_t *) req->handle,
                                       [](uv_handle_t *handle) {
                                           free(handle);
                                       });
                          }
                          free(req);
                      });

        if (r) LOG_UV_ERR("handle_dispatcher pipe write error", r);
    }

};