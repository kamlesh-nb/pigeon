#ifndef API_HANDLER_BASE_H
#define API_HANDLER_BASE_H

#include "http_msg.h"
#include "http_context.h"
#include <memory>
#include <http_parser.h>

namespace pigeon {

	class http_handler_base {

	public:

		virtual ~http_handler_base() {

		}

		virtual void get(http_context*) = 0;

		virtual void put(http_context*) = 0;

		virtual void post(http_context*) = 0;

		virtual void del(http_context*) = 0;

		virtual void process(http_context* context){

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



	template<typename T>
	inline std::shared_ptr<http_handler_base> create_api_handler() {
		return std::make_shared<T>();
	};


}



#endif //API_HANDLER_BASE_H