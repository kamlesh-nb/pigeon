#ifndef PIGEON_HTTP_HANDLERS_H
#define PIGEON_HTTP_HANDLERS_H

#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include "http_handler_base.h"

namespace pigeon {

	class http_handlers
	{

	private:

		http_handlers();
		static std::mutex _mtx;
		static std::shared_ptr<http_handlers> temp;
		std::unordered_map<string, http_handler_base*> handlers;

	public:

		~http_handlers();
		http_handlers(const http_handlers &handlers);

		void add(std::string, http_handler_base*);
		http_handler_base* get(std::string&);
		http_handler_base* get();

		static std::shared_ptr<http_handlers>& instance();

	};

}





#endif //PIGEON_HTTP_HANDLERS_H