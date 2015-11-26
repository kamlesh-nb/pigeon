//
// Created by kamlesh on 23/11/15.
//

#ifndef PIGEON_TCP_SERVER_H
#define PIGEON_TCP_SERVER_H

#include <asio.hpp>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "http_connection.h"
#include "io_contexts.h"
#include "http_handler_base.h"
#include "http_filter_base.h"

namespace pigeon {

    namespace tcp {

        class server {
        public:
            explicit server(std::string, std::string, std::size_t);

            void run();

        private:

			 

			 
            void do_accept();
			void do_await();
            void on_accept(const asio::error_code& e);
            void stop();
            
			io_contexts io_contexts_;
            asio::signal_set signals_;
            asio::ip::tcp::acceptor acceptor_;
            http_connnection_ptr new_http_connection_;

        };

    }

}



#endif //PIGEON_TCP_SERVER_H
