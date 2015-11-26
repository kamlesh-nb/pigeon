#ifndef PIGEON_HTTP_HANDLER_BASE_H
#define PIGEON_HTTP_HANDLER_BASE_H

#include "http_msg.h"
#include <http_parser.h>

namespace pigeon {

	class http_handler_base {



	public:

		virtual ~http_handler_base() {

		}

		virtual void get(http_request*) = 0;

		virtual void put(http_request*) = 0;

		virtual void post(http_request*) = 0;

		virtual void del(http_request*) = 0;

		virtual void process(http_request* request){

            switch (request->method)
            {
                case http_method::HTTP_GET:
					get(request);
                    break;

                case http_method::HTTP_POST:
					post(request);
                    break;

                case http_method::HTTP_PUT:
					put(request);
                    break;

                case http_method::HTTP_DELETE:
					del(request);
                    break;
            }

        }

	};

}



#endif //PIGEON_HTTP_HANDLER_BASE_H