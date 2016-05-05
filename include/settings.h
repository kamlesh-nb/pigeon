//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_APP_SETTINGS_H
#define PIGEON_APP_SETTINGS_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;


namespace pigeon {

    class settings {

    public:

        static string service_name;
        static int worker_threads;
        static string address;
        static int port;
        static string filters;
		static bool tcp_no_delay;
        static bool use_ssl;
        static string ssl_cert_file;
        static string ssl_key_file;
        static string document_root;
        static string resource_location;
        static string file_upload_location;
        static string default_page;
        static string log_location;
        static string db_conn_string;
        static string api_route;
        static bool enable_cors;
        static unordered_map<string, string> app_settings;
        static unordered_map<string, string> cors_headers;
        static vector<string> request_filters;
        static vector<string> response_filters;
        static vector<string> cors_allowed_origins;
        static vector<string> cors_allowed_methods;
        static auto load_setting() -> void;


    };

}


#endif //PIGEON_APP_SETTINGS_H
