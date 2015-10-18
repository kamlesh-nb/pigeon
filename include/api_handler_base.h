#ifndef API_HANDLER_BASE_H
#define API_HANDLER_BASE_H

#include "http_msg.h"
#include <memory>

namespace pigeon {

	class api_handler_base {

	public:

		virtual ~api_handler_base() {

		}

		virtual void get(const http_request &, http_response &) = 0;

		virtual void put(const http_request &, http_response &) = 0;

		virtual void post(const http_request &, http_response &) = 0;

		virtual void del(const http_request &, http_response &) = 0;

		virtual void process(const http_request &, http_response &) = 0;
	};

	template<typename T>
	inline std::shared_ptr<api_handler_base> create_api_handler() {
		return std::make_shared<T>();
	};


}



#endif //API_HANDLER_BASE_H