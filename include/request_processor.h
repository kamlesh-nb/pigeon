#ifndef PIGEON_REQUEST_PROCESSOR_H
#define PIGEON_REQUEST_PROCESSOR_H



#include <functional>
#include <memory>
#include <unordered_map>
#include <string>

#include "http_context.h"
#include "http_handler_base.h"
#include "http_filter_base.h"
#include "multi_part_parser.h"
#include "app_context.h"

using namespace std;


namespace pigeon {

	class request_processor
	{
	private:
		typedef std::shared_ptr<http_filter_base>(*CreateFilter)();
		typedef std::unordered_map<std::string, CreateFilter> filters;
		filters http_filters;

		typedef std::shared_ptr<http_handler_base>(*CreateHandler)();
		typedef std::unordered_map<std::string, CreateHandler> handlers;
		handlers http_handlers;


        string allowed_origins_cfg;
        string allowed_methods_cfg;
        string allowed_headers_cfg;
        string allowed_max_age_cfg;
        string allowed_credential_cfg;
        string allowed_expose_headers_cfg;

        string allowed_origins_hdr_fld;
        string allowed_methods_hdr_fld;
        string allowed_headers_hdr_fld;
        string allowed_max_age_hdr_fld;
        string allowed_credentials_hdr_fld;
        string allowed_expose_headers_hdr_fld;

        void parse_cookies(http_context*);
        void parse_url(http_context*);
        void parse_multipart(http_context*);
        bool execute_request_filters(http_context*);
        void handle_cors(http_context *);
        void handle_request(http_context*);
        bool execute_response_filters(http_context*);

        bool is_api(string &Uri, string &);
        multi_part_parser mpp;
	public:
		request_processor();
		~request_processor();
		void process(http_context*, std::function<void(http_context*)>);
	};

}



#endif //PIGEON_REQUEST_PROCESSOR_H
