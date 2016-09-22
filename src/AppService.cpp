//
// Created by kamlesh on 9/17/16.
//

#include "AppService.h"
#include "HttpServer.h"
using namespace pigeon;
using namespace pigeon::net;

HttpServer* _http_server;

void AppService::AddRoute(std::string route, int method, OnHandlerExecution func) {
    _http_server->AddRoute(route, method, func);
}
void AppService::AddFilter(std::string filter, OnHandlerExecution func) {
    _http_server->AddFilter(filter, func);
}
int AppService::Start() {
    _http_server->Start();
    return 0;
}
AppService::AppService() {
    _http_server = new HttpServer;
}
AppService::~AppService() {
    delete _http_server;
}

