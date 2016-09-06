//
// Created by kamlesh on 9/4/16.
//



#include "http_server.h"
#include "handle_dispatcher.ipp"

using namespace pigeon;


http_server::http_server() {
    _handle_dispatcher_impl = std::make_shared<handle_dispatcher>();
}
http_server::~http_server() {}

void http_server::start() {
    app_context::get()->load_settings();
    app_context::get()->load_cache();
    app_context::get()->add_handler("resource", createhandler<resource_handler>);
    _handle_dispatcher_impl->start();
}

