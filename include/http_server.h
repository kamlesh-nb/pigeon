//
// Created by kamlesh on 9/4/16.
//

#ifndef PIGEON_HTTP_SERVER_H
#define PIGEON_HTTP_SERVER_H

#include <uv.h>
#include <http_parser.h>
#include <memory>
#include <assert.h>

#include "app_context.h"
#include "http_context.h"
#include "request_processor.h"
#include "resource_handler.h"


#define LOG_UV_ERR(msg, code) do {                                                         \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);

namespace pigeon {

	typedef struct {
		uv_tcp_t stream;
		uv_loop_t* loop;
		http_parser parser;
		uv_write_t write_req;
		char *client_address;
		void *data;
		request_processor* reqproc_ptr;
		http_context *context;
		char *temp;
	} client_t;

    class http_server {
    private:
		class tls;
		class msg_parser;
        class listener;
		class tcp_listener;
		class tls_tcp_listener;

        class handle_dispatcher;
        std::shared_ptr<handle_dispatcher> _handle_dispatcher_impl;

    public:
        http_server();
        ~http_server();

        void start();

    };

}




#endif //PIGEON_HTTP_SERVER_H
