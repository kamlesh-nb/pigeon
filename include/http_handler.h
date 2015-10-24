//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_RESOURCE_HANDLER_H
#define PIGEON_RESOURCE_HANDLER_H

#include "http_context.h"
#include "http_handler_base.h"

namespace pigeon {

    class http_handler : public http_handler_base {

    private:

        string defaul_page;
        string resource_location;
        string cached_response;

    public:

        virtual ~http_handler();
        void get(http_context*) override;
        void post(http_context*) override;
        void put(http_context*) override;
        void del(http_context*) override;
        void options(http_context*);

        void process(http_context*) override;

    };

}

#endif //PIGEON_RESOURCE_HANDLER_H
