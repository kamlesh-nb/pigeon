#ifndef PIGEON_REQUEST_PROCESSOR_H
#define PIGEON_REQUEST_PROCESSOR_H


#include "http_context.h"
#include <functional>
#include "multi_part_parser.h"

namespace pigeon {

	class request_processor
	{
	private:
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
