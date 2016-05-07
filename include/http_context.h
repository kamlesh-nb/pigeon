//
// Created by kamlesh on 24/11/15.
//

#ifndef PIGEON_HTTP_CONTEXT_H
#define PIGEON_HTTP_CONTEXT_H

#include <http_msg.h>

namespace pigeon {

    struct http_context {

        http_request *request;
        http_response *response;
        void* data;
        http_context() {
            request = new http_request;
            response = new http_response;
        }

        ~http_context() {
            delete request;
            delete response;
        }

    };

}

#endif //PIGEON_HTTP_CONTEXT_H
