//
// Created by kamlesh on 23/11/15.
//

#include "tcp_server.h"

using namespace pigeon::tcp;

server::server(std::size_t io_context_pool_size)
        : io_contexts_(io_context_pool_size),
          signals_(io_contexts_.get_io_context()),
          acceptor_(io_contexts_.get_io_context()),
          new_connection_()

{

    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(
            [this](std::error_code /*ec*/, int /*signo*/)
            {
                acceptor_.close();
                handle_stop();

            });

    string address = "127.0.0.1";
    string port = "8080";

    asio::ip::tcp::resolver resolver(acceptor_.get_executor().context());
    asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    start_accept();
}

void server::run()
{
    io_contexts_.run();
}

void server::start_accept()
{
    new_connection_.reset(new connection(
            io_contexts_.get_io_context()));
    acceptor_.async_accept(new_connection_->socket(),
                           [this](std::error_code ec){
                               if (!acceptor_.is_open())
                               {
                                   return;
                               }

                               if (!ec)
                               {

                               }

                               handle_accept(ec);
                           });



}

void server::handle_accept(const asio::error_code& e)
{
    if (!e)
    {
        new_connection_->start();
    }

    start_accept();
}

void server::handle_stop()
{
    io_contexts_.stop();
}