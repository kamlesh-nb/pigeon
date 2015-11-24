//
// Created by kamlesh on 23/11/15.
//

#ifndef PIGEON_CONNECTION_H
#define PIGEON_CONNECTION_H

#include <asio.hpp>
#include <array>
#include <memory>
#include "http_msg.h"
#include "http_msg_parser.h"
#include "http_connection_base.h"

using namespace std;

namespace pigeon {

    namespace  tcp {

        class http_connection : public std::enable_shared_from_this<http_connection>, public http_connection_base
        {

        public:

            explicit http_connection(asio::io_context& io_context);

            void do_read();


        };

        typedef shared_ptr<http_connection> http_connnection_ptr;



    }

}



#endif //PIGEON_CONNECTION_H
