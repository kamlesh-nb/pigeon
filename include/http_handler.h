//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_HTTP_HANDLER_H
#define PIGEON_HTTP_HANDLER_H

#include "http_connection.h"
#include <http_handler_base.h>
#include <cache.h>

namespace pigeon {

    class http_handler : public http_handler_base {

    private:
        cache* m_cache;
        string default_page;
        string resource_location;
        string cached_response;

    public:

        http_handler();
        virtual ~http_handler();
        void get(http_context *) override;
        void post(http_context *) override;
        void put(http_context *) override;
        void del(http_context *) override;

        void process(http_context *) override;

        void options(http_context *);

    };

}

#endif //PIGEON_HTTP_HANDLER_H
