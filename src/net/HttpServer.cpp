//
// Created by kamlesh on 9/4/16.
//

#include "Settings.h"
#include "HttpServer.h"
#include "HandleDispatcher.ipp"

using namespace pigeon::net;

HttpServer::HttpServer() {
    cache_ptr = new FileCache;
    handle_dispatcher_impl = std::make_shared<HandleDispatcher>(cache_ptr);
    Settings::LoadSettings();
    handle_dispatcher_impl->CreateListeners();
}
HttpServer::~HttpServer() {}
void HttpServer::Start() {
    cache_ptr->Load(Settings::ResourceLocation);
    handle_dispatcher_impl->Start();
}
void HttpServer::AddRoute(std::string route, int method, OnHandlerExecution func){
    handle_dispatcher_impl->AddRoute(route, method, func);
}
void HttpServer::AddFilter(std::string filter, OnHandlerExecution func){
    handle_dispatcher_impl->AddFilter(filter, func);
}

