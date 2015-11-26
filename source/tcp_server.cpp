//
// Created by kamlesh on 23/11/15.
//

#include "tcp_server.h"
#include "settings.h"
#include "cache.h"
#include <iostream>

using namespace std;

using namespace pigeon::tcp;

server::server(std::string address, std::string port, std::size_t io_context_pool_size)
        : io_contexts_(io_context_pool_size),
          signals_(io_contexts_.get_io_context()),
          acceptor_(io_contexts_.get_io_context()),
          new_http_connection_()

{

    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    
	do_await();

    asio::ip::tcp::resolver resolver(acceptor_.get_executor().context());
	asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
    acceptor_.listen();

	do_accept();

}


void server::do_await(){

	signals_.async_wait(
		[this](std::error_code /*ec*/, int /*signo*/)
	{
		acceptor_.close();
		stop();

	});

}

void server::run()
{
	
	settings::load_setting();
	cache::get()->load(settings::resource_location);
    io_contexts_.run();

}

void server::do_accept()
{
    new_http_connection_.reset(new http_connection(
            io_contexts_.get_io_context()));
    acceptor_.async_accept(new_http_connection_->socket(),
                           [this](std::error_code ec){
                               if (!acceptor_.is_open())
                               {
                                   return;
                               }

                               if (!ec)
                               {
								   on_accept(ec);
                               }
                           });

}

void server::on_accept(const asio::error_code& e)
{
    if (!e)
    {
        new_http_connection_->do_read();
    }

	do_accept();
}

void server::stop()
{
    io_contexts_.stop();
}