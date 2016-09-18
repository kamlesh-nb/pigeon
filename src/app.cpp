//
// Created by kamlesh on 9/17/16.
//

#include "app.h"
#include "http_server.h"
using namespace pigeon;
using namespace pigeon::net;

http_server* _http_server;

void app::add_route(std::string route, int method, request_process_step_cb func) {
    _http_server->add_route(route, method,func);
}
void app::add_filter(std::string filter, request_process_step_cb func) {
    _http_server->add_filter(filter, func);
}
int app::start() {
    _http_server->start();
    return 0;
}
app::app() {
    _http_server = new http_server;
}
app::~app() {
    delete _http_server;
}

