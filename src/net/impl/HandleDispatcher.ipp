#include "Settings.h"
#include "HttpServer.h"
#include "TcpListener.ipp"
#include "TlsTcpListener.ipp"



using namespace pigeon;


class HttpServer::HandleDispatcher {
private:
    uv_loop_t *loop;
    uv_pipe_t* ipc_server;
    uv_tcp_t* tcp_server;
    FileCache* m_cache;
    uint  threads;

    struct sockaddr_in listen_addr;
    std::string pipe_name;
    std::vector<Listener*> listeners;

    void RemoveFSLink(){
        pipe_name = "/tmp/";
        pipe_name.append(Settings::ServiceName);

        uv_fs_t req;
        uv_fs_unlink(loop, &req, pipe_name.c_str(), NULL);
    }
    void InitTcp(){

        int r;

        string address = Settings::IPAddress;
        int port = Settings::Port;

        r = uv_ip4_addr(address.c_str(), port, &listen_addr);

        r = uv_tcp_init(loop, (uv_tcp_t*)tcp_server);
        if (r) LOG_UV_ERR("handle_dispatcher tcp initialize error", r);

        r = uv_tcp_nodelay((uv_tcp_t*)tcp_server, 1);

        r = uv_tcp_bind((uv_tcp_t*)tcp_server, (const struct sockaddr*) &listen_addr, 0);
        if (r) LOG_UV_ERR("handle_dispatcher tcp bind error", r);

    }
    void InitIpc(){

        int r;

        RemoveFSLink();
        ipc_server->data = this;
        uv_pipe_init(loop, ipc_server, 1);

        r = uv_pipe_bind(ipc_server, pipe_name.c_str());
        if (r) LOG_UV_ERR("handle_dispatcher pipe bind error", r);

        r = uv_listen((uv_stream_t*)ipc_server, 128,
                      [](uv_stream_t *server, int status){
                          HandleDispatcher* handle_dispatcher_ptr = static_cast<HandleDispatcher*>(server->data);
                          handle_dispatcher_ptr->OnIpcConnect(server, status);
                      });

        if (r) LOG_UV_ERR("handle_dispatcher pipe listen error", r);

    }

public:
    HandleDispatcher(FileCache* p_cache):m_cache(p_cache){}
    ~HandleDispatcher(){}
    void CreateListeners(){

        int threads = Settings::WorkerThreads;

        if(Settings::UseSsl){
            for (int i = 0; i < threads; ++i) {
                listeners.push_back(new TlsTcpListener(m_cache));
                //listeners[i]->create();
            }
        } else {
            for (int i = 0; i < threads; ++i) {
                listeners.push_back(new TcpListener(m_cache));
                //listeners[i]->create();
            }
        }

    }
    void Start(){

        loop = uv_default_loop();

        ipc_server = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
        tcp_server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

        for (auto& l : listeners) {
            l->Create();
        }

        InitTcp();
        InitIpc();

        for (auto& l : listeners) {
            if(l->ready)
                l->Post();
        }

        uv_run(loop, UV_RUN_DEFAULT);
        uv_close((uv_handle_t*)&ipc_server, NULL);

        uv_run(loop, UV_RUN_DEFAULT);

        for (auto& l : listeners) {
            if(l->ready)
                l->Wait();
        }
    }
    void AddRoute(std::string route, int method, OnHandlerExecution func){
        for(auto& l:listeners){
            l->AddRoute(route, method, func);
        }
    }
    void AddFilter(std::string filter, OnHandlerExecution func){
        for(auto& l:listeners){
            l->AddFilter(filter, func);
        }
    }
    void OnIpcConnect(uv_stream_t *server, int status){
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