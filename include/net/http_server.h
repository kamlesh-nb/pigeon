//
// Created by kamlesh on 9/4/16.
//

#ifndef PIGEON_HTTP_SERVER_H
#define PIGEON_HTTP_SERVER_H

#include <uv.h>
#include <http_parser.h>
#include <memory>
#include <assert.h>
#include <data/db_connection.h>
#include "app.h"
#include "cache.h"
#include "request_handler.h"

#define LOG_UV_ERR(msg, code) do {                                                      \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);

using namespace pigeon::data;

namespace pigeon {
	namespace net {
		typedef std::function<void(http_context*)> req_proc_done_cb;
		typedef struct {
            uv_tcp_t stream;
            uv_loop_t *loop;
            http_parser parser;
            uv_write_t write_req;
            char* client_address;
            void* data;
			db_connection* dbConnection;
            request_handler* requestHandler;
            http_context *context;
            char *temp;
        } client_t;

		class http_server {
		private:
            class msg_parser;
			class listener;
			class tcp_listener;
			class tls_tcp_listener;
			class handle_dispatcher;
            cache* cache_ptr;
			std::shared_ptr<handle_dispatcher> handle_dispatcher_impl;
		public:
			http_server();
			~http_server();
			void start();
            void add_route(std::string route, int method, request_process_step_cb func);
            void add_filter(std::string filter, request_process_step_cb func);
		};

	}

}




#endif //PIGEON_HTTP_SERVER_H
