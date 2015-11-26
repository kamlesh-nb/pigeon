//
// Created by kamlesh on 23/11/15.
//

#include "http_connection.h"
#include <vector>
#include <folly/futures/Future.h>
#include <http_msg_parser.h>

using namespace folly;
using namespace pigeon::tcp;

http_connection::http_connection(asio::io_context& io_context)
        : http_connection_base(io_context)
{

}



void http_connection::do_read()
{
    auto self(shared_from_this());
    socket().async_read_some(asio::buffer(buffer),
                            [this, self](std::error_code ec, std::size_t bytes_transferred)
                            {
                                if (!ec)
                                {
                                    parser->data = (void*)this;
                                    parse_request(bytes_transferred);
                                }
                                else if (ec != asio::error::operation_aborted)
                                {

                                }
                            });
}



