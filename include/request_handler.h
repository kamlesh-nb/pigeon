#ifndef HTTP_MSG_HANDLER_H
#define HTTP_MSG_HANDLER_H

#include "http_msg.h"
#include "app_constants.h"

namespace  pigeon {
	class request_handler {

	private:

		const char *err_msg1 = "<!DOCTYPE html><html><head lang='en'><meta charset='UTF-8'><title>Status</title></head><body><table><th>Status Code</th><th>Message</th><tr><td>";
		const char *err_msg3 = "</td><td>";
		const char *err_msg5 = "</td></tr></table></body></html>";

		string default_page;
		string resource_location;

		bool url_decode(const string &, string &);

		void process_resource_request(http_request &, http_response &);

		void process_api_request(http_request &, http_response &);

		void prepare(HttpStatus, http_request &, http_response &);

		void finish(HttpStatus, http_request &, http_response &);

	public:

		request_handler();

		~request_handler();

		void handle_request(http_request &, http_response &);
	};

}
#endif //HTTP_MSG_HANDLER_H
