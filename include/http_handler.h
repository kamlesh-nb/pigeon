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
        void get(http_connection *) override;
        void post(http_connection *) override;
        void put(http_connection *) override;
        void del(http_connection *) override;

        void process(http_connection *) override;

        void options(http_connection *);

    };

}

#endif //PIGEON_HTTP_HANDLER_H
