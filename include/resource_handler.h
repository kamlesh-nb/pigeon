//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_RESOURCE_HANDLER_H
#define PIGEON_RESOURCE_HANDLER_H

#include "http_handler_base.h"
#include "cache.h"

namespace pigeon {

    class resource_handler : public http_handler_base {

    private:

        string default_page;
        string resource_location;
        string cached_response;

    public:

        resource_handler();

        virtual ~resource_handler();

        void get(http_context *) override;

        void post(http_context *) override;

        void put(http_context *) override;

        void del(http_context *) override;

        void process(http_context *) override;

        void options(http_context *);

    };

}

#endif //PIGEON_RESOURCE_HANDLER_H
