#ifndef PIGEON_HTTP_HANDLER_BASE_H
#define PIGEON_HTTP_HANDLER_BASE_H

#include "http_context.h"
#include <http_parser.h>
using namespace pigeon::tcp;

namespace pigeon {

	class http_handler_base {


	protected:

		void prepare_response(http_context *);

	public:

		virtual ~http_handler_base() {

		}

		virtual void get(http_context *) = 0;

		virtual void put(http_context *) = 0;

		virtual void post(http_context *) = 0;

		virtual void del(http_context *) = 0;

		virtual void process(http_context * context){

            switch (context->request->method)
            {
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