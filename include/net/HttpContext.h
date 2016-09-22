//
// Created by kamlesh on 24/11/15.
//

#ifndef PIGEON_HTTP_CONTEXT_H
#define PIGEON_HTTP_CONTEXT_H

#include "HttpMessage.h"

namespace pigeon {
    namespace net {
        struct HttpContext {
            HttpRequest *Request;
            HttpResponse *Response;
            void* data;
            HttpContext() {
                Request = new HttpRequest;
                Response = new HttpResponse;
            }
            ~HttpContext() {
                delete Request;
                delete Response;
            }
        };
    }
}

#endif //PIGEON_HTTP_CONTEXT_H
