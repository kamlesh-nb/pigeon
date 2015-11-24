//
// Created by kamlesh on 23/11/15.
//

#ifndef PIGEON_CONNECTION_H
#define PIGEON_CONNECTION_H

#include <asio.hpp>
#include <array>
#include <memory>
#include "http_msg.h"

using namespace std;

namespace pigeon {

    namespace  tcp {

        class connection : public std::enable_shared_from_this<connection>
        {  
        public:
            /// Construct a connection with the given io_context.
            explicit connection(asio::io_context& io_context);

            /// Get the socket associated with the connection.
            asio::ip::tcp::socket& socket();

            /// Start the first asynchronous operation for the connection.
            void start();

        private:
            /// Handle completion of a read operation.
            void handle_read();

            /// Handle completion of a write operation.
            void handle_write(const asio::error_code& e);

            /// Socket for the connection.
            asio::ip::tcp::socket socket_;



            /// Buffer for incoming data.
            std::array<char, 65536> buffer_;

            /// The incoming request.
            http_request request_;

            /// The parser for the incoming request.
          //  request_parser request_parser_;

            /// The reply to be sent back to the client.
            http_response response_;
        };

        typedef shared_ptr<connection> connection_ptr;



    }

}



#endif //PIGEON_CONNECTION_H
