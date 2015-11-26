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

        class http_connection : public std::enable_shared_from_this<http_connection>
        {

		private:

			asio::ip::tcp::socket client;
			http_parser_settings parser_settings;
			http_parser* parser;
			shared_ptr<http_request> request;
			shared_ptr<http_response> response;
			std::array<char, 65536> buffer;
			
			
			void init_parser();
			void parse_request(size_t);
			

		public:
        
            explicit http_connection(asio::io_context& io_context);
			asio::ip::tcp::socket& socket();
            void do_read();
			void process_request();
			void do_write();


        };

        typedef shared_ptr<http_connection> http_connnection_ptr;



    }

}



#endif //PIGEON_CONNECTION_H
