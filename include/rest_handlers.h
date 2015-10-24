//
// Created by kamlesh on 24/10/15.
//

#ifndef PIGEON_REST_HANDLERS_H
#define PIGEON_REST_HANDLERS_H

#include <http_handler_base.h>
#include <mutex>

namespace pigeon {

    class rest_handlers {

    private:
        typedef std::shared_ptr<http_handler_base>(*CreateHandler)();
        typedef map<std::string, CreateHandler> handlers;
        handlers registry;

        static std::mutex _mtx;
        static std::shared_ptr<rest_handlers> instance;

        rest_handlers();

    public:

        rest_handlers(const rest_handlers &rhndlr);
        ~rest_handlers();


        auto add_handler(const std::string &, CreateHandler) -> void;
        auto get_handler(const std::string &) -> std::shared_ptr<http_handler_base> const;
        static std::shared_ptr<rest_handlers>& get();

    };

}


#endif //PIGEON_REST_HANDLERS_H
