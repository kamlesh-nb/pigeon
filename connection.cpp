//
// Created by kamlesh on 23/11/15.
//

#include "connection.h"
#include <vector>

using namespace pigeon::tcp;

connection::connection(asio::io_context& io_context)
        : socket_(io_context)
{
}

asio::ip::tcp::socket& connection::socket()
{
    return socket_;
}

void connection::start()
{
    handle_read();
}

void connection::handle_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(buffer_),
                            [this, self](std::error_code ec, std::size_t bytes_transferred)
                            {
                                if (!ec)
                                {

                                }
                                else if (ec != asio::error::operation_aborted)
                                {

                                }
                            });
}

void connection::handle_write(const asio::error_code& e)
{
    auto self(shared_from_this());
    asio::async_write(socket_,
                      response_.to_buffers(),
                      [this, self](std::error_code ec, std::size_t)
                      {
                          if (!ec)
                          {
                              // Initiate graceful connection closure.
                              asio::error_code ignored_ec;
                              socket_.shutdown(asio::ip::tcp::socket::shutdown_both,
                                               ignored_ec);
                          }

                          if (ec != asio::error::operation_aborted)
                          {

                          }
                      });
}
