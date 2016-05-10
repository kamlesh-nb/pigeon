#ifndef PIGEON_HTTP_HANDLERS_H
#define PIGEON_HTTP_HANDLERS_H

#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include "http_handler_base.h"

namespace pigeon {

    class http_handlers {
    private:
        http_handlers();
		typedef shared_ptr<http_handler_base>(*CreateHandler)();
		typedef unordered_map<std::string, CreateHandler> handlers;
		handlers registry;
        static std::mutex _mtx;
        static std::shared_ptr<http_handlers> temp;
    public:
        ~http_handlers();
        http_handlers(const http_handlers &handler);
        http_handlers& operator = (http_handlers const &);
		void add(const string&, CreateHandler);
		std::shared_ptr<http_handler_base> create(const std::string &) const;
        static std::shared_ptr<http_handlers> &instance();
    };

}


#endif //PIGEON_HTTP_HANDLERS_H