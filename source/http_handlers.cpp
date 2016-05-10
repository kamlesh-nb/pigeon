#include "http_handlers.h"

using namespace pigeon;


std::mutex http_handlers::_mtx;
std::shared_ptr<http_handlers> http_handlers::temp = nullptr;

http_handlers::http_handlers() {
}

http_handlers::~http_handlers() {
}

http_handlers::http_handlers(const http_handlers& handlers) {
    temp = handlers.temp;
}

http_handlers& http_handlers::operator = (const http_handlers& handlers) {
    if (this != &handlers) {
       temp = handlers.temp;
    }
    return *this;
}

void http_handlers::add(string handler_name, http_handler_base *handler) {
    handlers.emplace(std::pair<string, http_handler_base *>(handler_name, handler));
}

http_handler_base *http_handlers::get(string handler_name) {
    return handlers[handler_name];
}

void http_handlers::register_handler(const string& handler_name, CreateHandler func)
{
	registry[handler_name] = func;
}

std::shared_ptr<http_handler_base> http_handlers::Create(const std::string& handler_name) const
{
	std::shared_ptr<http_handler_base> _handler;
	handlers::const_iterator regEntry = registry.find(handler_name);
	if (regEntry != registry.end())
	{
		_handler = regEntry->second();
	}
	return _handler;
}

http_handler_base *http_handlers::get() {
    return handlers["resource"];
}

std::shared_ptr<http_handlers> &http_handlers::instance() {
    static std::shared_ptr<http_handlers> tmp = temp;

    if (!tmp) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!tmp) {
            temp.reset(new http_handlers);
            tmp = temp;
        }
    }

    return tmp;
}