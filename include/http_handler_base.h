#ifndef PIGEON_HTTP_HANDLER_BASE_H
#define PIGEON_HTTP_HANDLER_BASE_H


#include <http_parser.h>
#include "http_context.h"

namespace pigeon {

    class http_handler_base {


    public:

        virtual ~http_handler_base() {
        }

        virtual void get(http_context *context) {
			context->request->create_response("Not Implemented!", context->response, HttpStatus::NotImplemented);
		}
        virtual void put(http_context *context) {
			context->request->create_response("Not Implemented!", context->response, HttpStatus::NotImplemented);
		}
        virtual void post(http_context *context) {
			context->request->create_response("Not Implemented!", context->response, HttpStatus::NotImplemented);
		}
        virtual void del(http_context *context) {
			context->request->create_response("Not Implemented!", context->response, HttpStatus::NotImplemented);
		}
        virtual void process(http_context *context) {

            switch (context->request->method) {
                case http_method::HTTP_GET:
                    get(context);
                    break;

                case http_method::HTTP_POST:
                    post(context);
                    break;

                case http_method::HTTP_PUT:
                    put(context);
                    break;

                case http_method::HTTP_DELETE:
                    del(context);
                    break;
            }

        }

    };

}


#endif //PIGEON_HTTP_HANDLER_BASE_H