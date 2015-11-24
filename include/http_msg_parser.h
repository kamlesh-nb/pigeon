//
// Created by kamlesh on 23/11/15.
//

#ifndef PIGEON_HTTP_MSG_PARSER_H
#define PIGEON_HTTP_MSG_PARSER_H

#include <http_parser.h>
#include "http_connection.h"

namespace pigeon {

    namespace tcp {

        class http_msg_parser {

        private:
            http_parser* parser;
            http_parser_settings parser_settings;
            bool is_api(const string&);
            void parse_query_string(http_request&);

        public:
            http_msg_parser();
            ~http_msg_parser();
            void init();
            void parse_request(char*, asio::ip::tcp::socket, size_t);

        };

    }

}


#endif //PIGEON_HTTP_MSG_PARSER_H
