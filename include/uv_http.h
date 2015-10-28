#ifndef HTTP_H
#define HTTP_H
#include <uv.h>
#include <http_parser.h>
 
#include <assert.h>
#include <exception>
#include <http_context.h>
#include <logger.h>
#include <iostream>

#include "stop_watch.h"

using namespace std;

namespace pigeon {

	namespace http {

		uv_loop_t* uv_loop;
		uv_tcp_t uv_tcp;
		http_parser_settings parser_settings;

		settings* _Settings;
		cache* _Cache;

		typedef struct {

			uv_tcp_t handle;
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

		struct oconnection_t {
			oconnection_t() :
			body() {}
			http_parser parser;
			int request_num;
			uv_tcp_t tcp;
			uv_connect_t connect_req;
			uv_shutdown_t shutdown_req;
			uv_write_t write_req;
			std::stringstream body;
		};

		namespace server {

			auto on_alloc(uv_handle_t * /*handle*/, size_t suggested_size, uv_buf_t* buf) -> void {
				*buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
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

			auto on_send_complete(uv_write_t *req, int status) -> void {

				try {

					if (status != 0){
						logger::get(_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(status));
					}
					if (!uv_is_closing((uv_handle_t*)req->handle))
					{
						msg_baton_t *closure = static_cast<msg_baton_t *>(req->data);
						delete closure;
						uv_close((uv_handle_t*)req->handle, on_close);
					}

				}
				catch (std::exception& ex){
					throw std::runtime_error(ex.what());
				}

			}

			auto on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf) -> void {
				try {

					ssize_t parsed;
					iconnection_t *iConn = (iconnection_t *)tcp->data;
					if (nread >= 0) {
						parsed = (ssize_t)http_parser_execute(&iConn->parser, &parser_settings, buf->base, nread);
						if (parsed < nread) {
							logger::get(_Settings)->write(LogType::Error, Severity::Critical, "parse failed");
							uv_close((uv_handle_t *)&iConn->handle, on_close);
						}
					}
					else {
						if (nread != UV_EOF) {
							logger::get(_Settings)->write(LogType::Error, Severity::Critical, "read failed");
						}
						uv_close((uv_handle_t *)&iConn->handle, on_close);
					}
					free(buf->base);
				}
				catch (std::exception& ex){

					throw std::runtime_error(ex.what());

				}
			}

			auto on_connect(uv_stream_t* server_handle, int status) -> void {
				try {

					assert((uv_tcp_t*)server_handle == &uv_tcp);

					iconnection_t* iConn = (iconnection_t*)malloc(sizeof(iconnection_t));

					iConn->context = new http_context;
					iConn->context->Settings = _Settings;
					iConn->context->Cache = _Cache;


					uv_tcp_init(uv_loop, &iConn->handle);
					http_parser_init(&iConn->parser, HTTP_REQUEST);

					iConn->parser.data = iConn;
					iConn->handle.data = iConn;

					int r = uv_accept(server_handle, (uv_stream_t*)&iConn->handle);
                    if (r != 0){
                        logger::get(_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
                    }

                    struct sockaddr_in name;
                    int namelen = sizeof(name);
                    r = uv_tcp_getpeername(&iConn->handle, (struct sockaddr*) &name, &namelen);
                    if (r != 0){
                        logger::get(_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
                    }

                    char addr[16];
                    char buf[32];
                    uv_inet_ntop(AF_INET, &name.sin_addr, addr, sizeof(addr));
                    snprintf(buf, sizeof(buf), "%s:%d", addr, ntohs(name.sin_port));
                    iConn->client_adress = buf;

					if (r != 0){
						logger::get(_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
					}

					uv_read_start((uv_stream_t*)&iConn->handle, on_alloc, on_read);


				}
				catch (std::exception& ex){
					throw std::runtime_error(ex.what());
				}
			}

			auto on_render(uv_work_t *req) -> void {
				try {

					msg_baton_t *closure = static_cast<msg_baton_t *>(req->data);
					iconnection_t* iConn = (iconnection_t*)closure->iConn;

					process(iConn->context);

					closure->result = (char*)iConn->context->response->message.c_str();
					closure->length = iConn->context->response->message.size();

				}
				catch (std::exception& ex) {
					throw std::runtime_error(ex.what());
				}
			}

			auto on_render_complete(uv_work_t *req, int status) -> void {
				try {

					msg_baton_t *closure = static_cast<msg_baton_t *>(req->data);
					iconnection_t* iConn = (iconnection_t*)closure->iConn;


					uv_buf_t resbuf;
					resbuf.base = closure->result;
					resbuf.len = (unsigned long)closure->length;

					iConn->write_req.data = closure;

					int r = uv_write(&iConn->write_req,
						(uv_stream_t*)&iConn->handle,
						&resbuf,
						1,
						on_send_complete);

					if (r != 0){
						logger::get(_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
					}

				}
				catch (std::exception& ex) {
					throw std::runtime_error(ex.what());
				}
			}

		}

		namespace iConn {
			
		}

		namespace parser {

			auto on_url(http_parser* parser, const char* at, size_t len) -> int {

				iconnection_t* iConn = (iconnection_t*)parser->data;
				if (at && iConn->context->request) {
					//string s(at, len);
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
				msg_baton_t *closure = new msg_baton_t();
				closure->request.data = closure;
				closure->iConn = iConn;
				closure->error = false;
				
				iConn->context->request->is_api = is_api(iConn->context->request->url);
				parse_query_string(*iConn->context->request);

				int status = uv_queue_work(uv_loop,
					&closure->request,
					server::on_render,
					server::on_render_complete);

				return status;

			}

		}

	}
}

#endif //HTTP_H
