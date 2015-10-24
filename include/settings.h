//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_APP_SETTINGS_H
#define PIGEON_APP_SETTINGS_H
#include <string>
#include <vector>
#include <key_value_pair.h>
#include <map>

using namespace std;


namespace pigeon {

    class settings {
    private:




        string service_name;
        string address;
        int port;
        bool use_ssl;
        string ssl_cert_file;
        string ssl_key_file;
        int num_worker_threads;
        string document_root;
        string resource_location;
        string file_upload_location;
        string default_page;
        string log_location;
        string db_conn_string;
        string api_route;

        vector<key_value_pair> appsettings;
        vector<key_value_pair> corsheaders;

    public:

        friend class logger;

        auto get_address() -> string&;

        auto get_port() -> int;

        auto get_api_route() -> string&;

        auto get_db_conn_str() -> string&;

        auto get_default_page() -> string&;

        auto get_document_root() -> string&;

        auto get_log_location() -> string&;

        auto get_num_worker_threads() -> int;

        auto get_resource_location() -> string&;

        auto get_file_upload_location() -> string&;

        auto get_service_name() -> string&;

        auto get_ssl_cert_file() -> string&;

        auto get_ssl_key_file() -> string&;

        auto get_use_ssl() -> bool;

        auto set_app_setting_key(string& _key) -> void;

        auto set_app_setting_value(string& _value) -> void;

        auto get_app_setting(key_value_pair& kvp) -> void;

        auto set_corsheaders_key(string& _key) -> void;

        auto set_corsheaders_value(string& _value) -> void;

        auto get_corsheaders(key_value_pair& kvp) -> void;

        auto load_setting() -> void;


    };

}



#endif //PIGEON_APP_SETTINGS_H
