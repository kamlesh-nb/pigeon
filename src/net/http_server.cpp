//
// Created by kamlesh on 9/4/16.
//



#include "settings.h"
#include "http_server.h"
#include "handle_dispatcher.ipp"


using namespace pigeon::net;


http_server::http_server() {
    cache_ptr = new cache;
    handle_dispatcher_impl = std::make_shared<handle_dispatcher>(cache_ptr);
    settings::load_setting();
    handle_dispatcher_impl->create_listeners();
}
http_server::~http_server() {}

void http_server::start() {

    cache_ptr->load(settings::resource_location);
    handle_dispatcher_impl->start();
}

void http_server::add_route(std::string route, int method, request_process_step_cb func){

    handle_dispatcher_impl->add_route(route, method, func);

}

void http_server::add_filter(std::string filter, request_process_step_cb func){

    handle_dispatcher_impl->add_filter(filter, func);

}

