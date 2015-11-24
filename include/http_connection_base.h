//
// Created by kamlesh on 24/11/15.
//

#ifndef PIGEON_HTTP_CONTEXT_BASE_H
#define PIGEON_HTTP_CONTEXT_BASE_H

#include <asio/ip/tcp.hpp>
#include <http_parser.h>
#include "http_msg.h"
#include "http_context.h"

namespace pigeon {

    namespace tcp {

        class http_connection_base {


        private:

            asio::ip::tcp::socket client;
            http_parser* parser;
            http_parser_settings parser_settings;
            void init_parser();

        protected:

            shared_ptr<http_context> context;

            std::array<char, 65536> buffer;
            void do_write();

            void parse_request(size_t);

        public:

            explicit http_connection_base(asio::io_context& io_context);
            asio::ip::tcp::socket& socket();

        };

    }
}




#endif //PIGEON_HTTP_CONTEXT_BASE_H
