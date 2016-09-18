//
// Created by kamlesh on 9/17/16.
//

#ifndef PIGEON_REQUEST_HANDLER_H
#define PIGEON_REQUEST_HANDLER_H

#include "http_context.h"
#include "multi_part_parser.h"
#include "app.h"

namespace pigeon {

    namespace net {

        class request_handler {

        private:
            multi_part_parser mpp;
            cache* m_cache;

            std::unordered_map<std::string, request_process_step_cb> get_func;
            std::unordered_map<std::string, request_process_step_cb> put_func;
            std::unordered_map<std::string, request_process_step_cb> post_func;
            std::unordered_map<std::string, request_process_step_cb> del_func;
            std::unordered_map<std::string, request_process_step_cb> filter_func;

            //cors headers
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
            //cors headers

            //resource
            string default_page;
            string resource_location;
            //resource
        public:
            request_handler(cache*);
            ~request_handler();

            int handle_request(http_context *context);
            int parse_multipart(http_context *context);
            int execute_request_filters(http_context *context);
            int execute_response_filters(http_context *context);
            int handle_cors(http_context *context);
            int parse_cookies(http_context *context);
            int parse_url(http_context *context);
            bool is_api(string &Uri, string &apiroute);
            int process(http_context *context);
            int finish_process(http_context* context);
            int get_resource(http_context *context);
            void add_route(std::string route, int method, request_process_step_cb func);
            void add_filter(std::string filter, request_process_step_cb func);
        };


    }
}


#endif //PIGEON_REQUEST_HANDLER_H
