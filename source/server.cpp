//
// Created by kamlesh on 17/10/15.
//

#include <uv.h>
#include <stdio.h>
#include <malloc.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <http_parser.h>
#include <string.h>
#include <http_util.h>
#include <logger.h>
#include <server.h>
#include <http_context.h>
#include <http_handlers.h>
#include <cache.h>
#include <resource_handler.h>
#include <unistd.h>
#include <vector>
#include <http_filters.h>


#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-offsetof(type, member)))

#define MAX_WRITE_HANDLES 1000

namespace pigeon {


    http_parser_settings parser_settings;

    typedef struct {
        uv_tcp_t handle;
        uv_shutdown_t shutdown_req;
    } conn_rec_t;

    typedef struct {

        uv_tcp_t handle;
        http_parser parser;
        uv_write_t write_req;
        char* client_adress;
        void* data;
        http_context* context;

    } iconnection_t;

    typedef struct {

        uv_work_t request;
        iconnection_t* iConn;
        bool error;
        char* result;
        size_t length;

    } msg_baton_t;

    class server::server_impl {

    private:

		uv_loop_t* uv_loop;
		uv_tcp_t uv_tcp;
        vector<string> filters;

        void _init(){

            settings::load_setting();
            cache::get()->load(settings::resource_location);

            http_handlers::instance()->add("resource", new resource_handler());

            uv_loop = uv_default_loop();

        }

        void _tcp(){

            int r = uv_tcp_init(uv_loop, &uv_tcp);

            if (r != 0){
                logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
            }

            r = uv_tcp_keepalive(&uv_tcp, 1, 60);
            if (r != 0){
                logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
            }

        }

        void _bind(){

            string _address = settings::address;
            int _port = settings::port;

            struct sockaddr_in address;
            int r = uv_ip4_addr(_address.c_str(), _port, &address);
            if (r != 0){
                logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
            }

            r = uv_tcp_bind(&uv_tcp, (const struct sockaddr*)&address, 0);
            if (r != 0){
                logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
            }

        }

        void _listen(){

            uv_tcp.data = this;

            int r = uv_listen((uv_stream_t*)&uv_tcp, MAX_WRITE_HANDLES,
                              [](uv_stream_t *socket, int status) {
                                  server_impl* srvImpl = static_cast<server_impl*>(socket->data);
                                  srvImpl->on_connect(socket, status);
                              });

            if (r != 0){
                logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
            }

            logger::get()->write(LogType::Information, Severity::Medium, "Server Started...");

            uv_run(uv_loop, UV_RUN_DEFAULT);

        }

        void _parser() {

            parser_settings.on_url = [](http_parser* parser, const char* at, size_t len) -> int {

                iconnection_t* iConn = (iconnection_t*)parser->data;
                if (at && iConn->context->request) {
                    char *data = (char *)malloc(sizeof(char) * len + 1);
                    strncpy(data, at, len);
                    data[len] = '\0';
                    iConn->context->request->url += data;
                    free(data);
                }
                return 0;

            };

            parser_settings.on_header_field = [](http_parser* parser, const char* at, size_t len) -> int {

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

            };

            parser_settings.on_header_value = [](http_parser* parser, const char* at, size_t len) -> int {

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

            };

            parser_settings.on_headers_complete = [](http_parser* parser) -> int {

                iconnection_t* iConn = (iconnection_t*)parser->data;
                iConn->context->request->method = parser->method;
                iConn->context->request->http_major_version = parser->http_major;
                iConn->context->request->http_minor_version = parser->http_minor;
                return 0;

            };

            parser_settings.on_body = [](http_parser* parser, const char* at, size_t len) -> int {

                iconnection_t* iConn = (iconnection_t*)parser->data;
                if (at && iConn->context->request) {

                    char *data = (char *)malloc(sizeof(char) * len + 1);
                    strncpy(data, at, len);
                    data[len] = '\0';
                    iConn->context->request->content += data;
                    free(data);

                }
                return 0;

            };

            parser_settings.on_message_complete = [](http_parser* parser) -> int {

                iconnection_t* iConn = (iconnection_t*)parser->data;
                msg_baton_t *closure = new msg_baton_t();
                closure->request.data = closure;
                closure->iConn = iConn;
                closure->error = false;

                int status = uv_queue_work(uv_default_loop(),
                                           &closure->request,
                                           [](uv_work_t* req){
                                               msg_baton_t* bton =  static_cast<msg_baton_t*>(req->data);
                                               server_impl* srvImpl = static_cast<server_impl*>(bton->iConn->data);
                                               srvImpl->on_process(req);
                                           },
                                           [](uv_work_t* req, int status){
                                               msg_baton_t* bton =  static_cast<msg_baton_t*>(req->data);
                                               server_impl* srvImpl = static_cast<server_impl*>(bton->iConn->data);
                                               srvImpl->on_process_complete(req);
                                           });

                return 0;

            };
        }

    public:

		void on_shutdown(uv_handle_t* req, int status) {
			if (!uv_is_closing((uv_handle_t*)req)) {
				uv_close((uv_handle_t*)req, [](uv_handle_t *handle){
					server_impl* srvImpl = static_cast<server_impl*>(handle->data);
					srvImpl->on_close((uv_handle_t *)&handle);
				});
			}
			free(req);
		}

        void on_process(uv_work_t* req){

            msg_baton_t* closure =  static_cast<msg_baton_t*>(req->data);
            iconnection_t* iConn = closure->iConn;


            iConn->context->request->is_api = is_api(iConn->context->request->url);
            parse_query_string(*iConn->context->request);

            server_impl* srvImpl = static_cast<server_impl*>(iConn->data);

            srvImpl->process(iConn->context);

            closure->result = (char*)iConn->context->response->message.c_str();
            closure->length = iConn->context->response->message.size();

        }

        void on_process_complete(uv_work_t* req){

            msg_baton_t* closure =  static_cast<msg_baton_t*>(req->data);
            iconnection_t* iConn = closure->iConn;

            uv_buf_t resbuf;
            resbuf.base = closure->result;
            resbuf.len = (unsigned long)closure->length;

            iConn->write_req.data = closure;

            int r = uv_write(&iConn->write_req,
                             (uv_stream_t*)&iConn->handle,
                             &resbuf,
                             1,
                             [](uv_write_t *req, int status) {
                                 msg_baton_t *closure = static_cast<msg_baton_t *>(req->data);
                                 server_impl* srvImpl = static_cast<server_impl*>(closure->iConn->data);
                                 srvImpl->on_send_complete(req, status);
                             });

            if (r != 0){
                logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
            }

        }

        void on_close(uv_handle_t *handle) {

            try {

                iconnection_t* iConn = (iconnection_t*)handle->data;
                delete iConn->context;
                free(iConn);

            }
            catch (std::exception ex) {

            }
        }

        void on_send_complete(uv_write_t *req, int status) {

            try {

                if (status != 0){
                    logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(status));
                }

				if (!uv_is_closing((uv_handle_t*)req->handle))
				{
					msg_baton_t *closure = static_cast<msg_baton_t *>(req->data);
					delete closure;
					uv_close((uv_handle_t*)req->handle, [](uv_handle_t *handle){
						server_impl* srvImpl = static_cast<server_impl*>(handle->data);
						srvImpl->on_close((uv_handle_t *)&handle);
					});
				}
				 
            }
            catch (std::exception& ex){
                throw std::runtime_error(ex.what());
            }

        }

        void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf) {
            try {

                ssize_t parsed;
                iconnection_t *iConn = (iconnection_t *)tcp->data;

                if (nread >= 0) {

                    parsed = (ssize_t)http_parser_execute(&iConn->parser, &parser_settings, buf->base, nread);
                    if (parsed < nread) {
                        logger::get()->write(LogType::Error, Severity::Critical, "parse failed");
                        uv_close((uv_handle_t *)&iConn->handle, [](uv_handle_t *handle){
                            server_impl* srvImpl = static_cast<server_impl*>(handle->data);
                            srvImpl->on_close((uv_handle_t *)&handle);
                        });
                    }
                }
                else {
                    if (nread != UV_EOF) {
                        logger::get()->write(LogType::Error, Severity::Critical, "read failed");
                    }
                    uv_close((uv_handle_t *)&iConn->handle, [](uv_handle_t *handle){
                        server_impl* srvImpl = static_cast<server_impl*>(handle->data);
                        srvImpl->on_close((uv_handle_t *)&handle);
                    });
                }
                free(buf->base);
            }
            catch (std::exception& ex){

                throw std::runtime_error(ex.what());

            }
        }

        void on_connect(uv_stream_t* server_handle, int status) {
            try {

                assert((uv_tcp_t*)server_handle == &uv_tcp);

                iconnection_t* iConn = (iconnection_t*)malloc(sizeof(iconnection_t));

                iConn->context = new http_context;

                iConn->data = server_handle->data;


                uv_tcp_init(uv_loop, &iConn->handle);
                http_parser_init(&iConn->parser, HTTP_REQUEST);

                iConn->parser.data = iConn;
                iConn->handle.data = iConn;

                int r = uv_accept(server_handle, (uv_stream_t*)&iConn->handle);
                if (r != 0){
                    logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
                }
                uv_tcp_nodelay((uv_tcp_t*)&server_handle, 1);

                struct sockaddr_in name;
                int namelen = sizeof(name);
                r = uv_tcp_getpeername(&iConn->handle, (struct sockaddr*) &name, &namelen);
                if (r != 0){
                    logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
                }

                char addr[16];
                char buf[32];
                uv_inet_ntop(AF_INET, &name.sin_addr, addr, sizeof(addr));
#ifdef _WIN32
                _snprintf(buf, sizeof(buf), "%s:%d", addr, ntohs(name.sin_port));
#else
                snprintf(buf, sizeof(buf), "%s:%d", addr, ntohs(name.sin_port));
#endif
                iConn->client_adress = buf;

                if (r != 0){
                    logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
                }

                uv_read_start((uv_stream_t*)&iConn->handle, [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
                    *buf = uv_buf_init((char *) malloc(suggested_size), suggested_size);
                }, [](uv_stream_t *socket, ssize_t nread, const uv_buf_t *buf) {
                    server_impl* srvImpl = static_cast<server_impl*>(socket->data);
                    srvImpl->on_read(socket, nread, buf);
                });


            }
            catch (std::exception& ex){
                throw std::runtime_error(ex.what());
            }
        }

        void start() {


#ifndef _WIN32
            signal(SIGPIPE, SIG_IGN);
#endif



            _init();

#ifdef _WIN32
            string num_of_threads = std::to_string(settings::worker_threads);
		    SetEnvironmentVariable(VARNAME, (LPTSTR)num_of_threads.c_str());
#else
            stringstream ss;
            ss << settings::worker_threads;
            setenv("UV_THREADPOOL_SIZE", ss.str().c_str(), 1);
#endif
            //load filter tokens
            int i = 0;
            stringstream ss_filters(settings::filters);
            while (ss_filters.good() && i < 4){
                string filt;
                ss_filters >> filt;
                filters.push_back(filt);
                ++i;
            }
            //done


            _parser();
            _tcp();
            _bind();
            _listen();

        }

        void process(http_context *context) {

            for(auto& flt:filters){

                if(flt.length() > 0){
                    auto filter = http_filters::instance()->get(flt);
                    filter->init();
                    filter->execute(context);
                    filter->clean();

                }

            }

            if(context->request->is_api){
                auto handler = http_handlers::instance()->get(context->request->url);
                handler->process(context);
            } else {
                auto handler = http_handlers::instance()->get();
                handler->process(context);
            }

        }


    };

	server::server() {
        _Impl = new server_impl;
    }

	server::~server() { }

	void server::start() {

        _Impl->start();
	}




}