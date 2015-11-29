//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_HTTP_HANDLER_H
#define PIGEON_HTTP_HANDLER_H

#include "http_connection.h"
#include <http_handler_base.h>
#include <cache.h>

namespace pigeon {

    class resource_handler : public http_handler_base {

    private:
        
		string default_page;
        string resource_location;
        string cached_response;

    public:

        resource_handler();
        virtual ~resource_handler();
		void get(http_request *) override;
		void post(http_request *) override;
		void put(http_request *) override;
		void del(http_request *) override;

		void process(http_request *) override;

		void options(http_request *);

    };

}

#endif //PIGEON_HTTP_HANDLER_H
