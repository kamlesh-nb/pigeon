//
// Created by kamlesh on 9/4/16.
//

#ifndef PIGEON_HTTP_SERVER_H
#define PIGEON_HTTP_SERVER_H

#include <uv.h>
#include <http_parser.h>
#include <memory>
#include <assert.h>
#include <data/IDbConnection.h>
#include "AppService.h"
#include "FileCache.h"
#include "RequestHandler.h"

#define LOG_UV_ERR(msg, code) do {                                                      \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);

using namespace pigeon::data;

namespace pigeon {
	namespace net {
		typedef std::function<void(HttpContext*)> OnRequestProcessingComplete;
		typedef struct {
            uv_tcp_t stream;
            uv_loop_t *loop;
            http_parser parser;
            uv_write_t write_req;
            char* client_address;
            void* data;
			IDbConnection* dbConnection;
            RequestHandler* requestHandler;
            HttpContext *context;
            char *temp;
        } client_t;

		class HttpServer {
		private:
            class MessageParser;
			class Listener;
			class TcpListener;
			class TlsTcpListener;
			class HandleDispatcher;
            FileCache* cache_ptr;
			std::shared_ptr<HandleDispatcher> handle_dispatcher_impl;
		public:
			HttpServer();
			~HttpServer();
			void Start();
            void AddRoute(std::string route, int method, OnHandlerExecution func);
            void AddFilter(std::string filter, OnHandlerExecution func);
		};

	}

}




#endif //PIGEON_HTTP_SERVER_H
