//
// Created by kamlesh on 24/11/15.
//

#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include "http_connection_base.h"

using namespace pigeon::tcp;


http_connection_base::http_connection_base(asio::io_context &io_context) : client(io_context){
    context = make_shared<http_context>();
}

