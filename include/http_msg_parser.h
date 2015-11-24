//
// Created by kamlesh on 23/11/15.
//

#ifndef PIGEON_HTTP_MSG_PARSER_H
#define PIGEON_HTTP_MSG_PARSER_H

#include <http_parser.h>

namespace pigeon {

    namespace tcp {

        class http_msg_parser {

        private:
            http_parser* parser;
            http_parser_settings parser_settings;

        public:
            http_msg_parser();
            ~http_msg_parser();
            void init();
            void parse_request();

        };

    }

}


#endif //PIGEON_HTTP_MSG_PARSER_H
