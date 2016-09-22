//
// Created by kamlesh on 9/17/16.
//

#ifndef PIGEON_REQUEST_HANDLER_H
#define PIGEON_REQUEST_HANDLER_H

#include "HttpContext.h"
#include "MultiPartParser.h"
#include "AppService.h"

namespace pigeon {

    namespace net {

        class RequestHandler {

        private:
            MultiPartParser mpp;
            FileCache* m_cache;

            std::unordered_map<std::string, OnHandlerExecution> get_func;
            std::unordered_map<std::string, OnHandlerExecution> put_func;
            std::unordered_map<std::string, OnHandlerExecution> post_func;
            std::unordered_map<std::string, OnHandlerExecution> del_func;
            std::unordered_map<std::string, OnHandlerExecution> filter_func;

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
            RequestHandler(FileCache*);
            ~RequestHandler();

            int HandleRequest(HttpContext *context);
            int MarseMultiPartData(HttpContext *context);
            int ExecuteRequestFilters(HttpContext *context);
            int ExecuteResponseFilters(HttpContext *context);
            int HandleCORSHeaders(HttpContext *context);
            int ParseCookies(HttpContext *context);
            int ParseUrl(HttpContext *context);
            bool IsApi(string &Uri, string &apiroute);
            int StartProcess(HttpContext *context);
            int FinishProcess(HttpContext *context);
            int GetResource(HttpContext *context);
            void AddRoute(std::string route, int method, OnHandlerExecution func);
            void AddFilter(std::string filter, OnHandlerExecution func);
        };


    }
}


#endif //PIGEON_REQUEST_HANDLER_H
