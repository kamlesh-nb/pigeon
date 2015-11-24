//
// Created by kamlesh on 23/11/15.
//

#ifndef PIGEON_TCP_SERVER_H
#define PIGEON_TCP_SERVER_H

#include <asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "http_connection.h"
#include "io_contexts.h"


namespace pigeon {

    namespace tcp {

        class server {
        public:
            /// Construct the server to listen on the specified TCP address and port, and
            /// serve up files from the given directory.
            explicit server(std::size_t);

            /// Run the server's io_context loop.
            void run();

        private:
            /// Initiate an asynchronous accept operation.
            void start_accept();

            /// Handle completion of an asynchronous accept operation.
            void handle_accept(const asio::error_code& e);

            /// Handle a request to stop the server.
            void handle_stop();

            /// The pool of io_context objects used to perform asynchronous operations.
            io_contexts io_contexts_;

            /// The signal_set is used to register for process termination notifications.
            asio::signal_set signals_;

            /// Acceptor used to listen for incoming connections.
            asio::ip::tcp::acceptor acceptor_;

            /// The next http_connection to be accepted.
            http_connnection_ptr new_http_connection_;

        };

    }

}



#endif //PIGEON_TCP_SERVER_H
