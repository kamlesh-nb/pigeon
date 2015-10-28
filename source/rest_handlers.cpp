//
// Created by kamlesh on 24/10/15.
//

#include <iostream>
#include "rest_handlers.h"

using namespace pigeon;


auto rest_handlers::add_handler(const string &handler_name, CreateHandler handler) -> void {
    registry[handler_name] = handler;
}

auto rest_handlers::get_handler(const string &handler_name) -> std::shared_ptr<http_handler_base> const {

    std::shared_ptr<http_handler_base> op_handler = nullptr;
    handlers::const_iterator regEntry = registry.find(handler_name);
    if (regEntry != registry.end())
    {
        op_handler = regEntry->second();
    }
    return op_handler;

}

rest_handlers::rest_handlers(const rest_handlers &rhndlr) {

}

rest_handlers::~rest_handlers() {

}

std::shared_ptr<rest_handlers> rest_handlers::instance = nullptr;

std::mutex rest_handlers::_mtx;

std::shared_ptr<rest_handlers> &rest_handlers::get() {
    static std::shared_ptr<rest_handlers> tmp = instance;

    if (!tmp)
    {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!tmp)
        {
            instance.reset(new rest_handlers);
            tmp = instance;
        }
    }

    return tmp;
}

rest_handlers::rest_handlers() {

}
