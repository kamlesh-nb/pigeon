#include "app_context.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;
using namespace pigeon;
using namespace rapidjson;

class app_context::settings {

private:
    string service_name;
    int worker_threads;
    string address;
    int port;
    string filters;
    bool  tcp_no_delay;
    bool use_ssl;
    string ssl_cert_file;
    string ssl_key_file;
    string document_root;
    string resource_location;
    string file_upload_location;
    string default_page;
    string log_location;
    string db_conn_string;
    string api_route;
    bool enable_cors;
    unordered_map<string, string> app_settings;
    unordered_map<string, string> cors_headers;
    vector<string> request_filters;
    vector<string> response_filters;
    vector<string> cors_allowed_origins;
    vector<string> cors_allowed_methods;

public:

    settings() {
        auto x = this;
    }
    ~settings() {}

    void load_settings(){

            char path[1024];
            size_t sz = 1024;
            uv_cwd(path, &sz);
            string current_path(path);

            current_path.append("/service.json");

            std::ifstream is(current_path.c_str(), std::ios::in | std::ios::binary);
            std::string content((std::istreambuf_iterator<char>(is)), (std::istreambuf_iterator<char>()));

            Document doc;
            doc.Parse(content.c_str());

            service_name = doc["service_name"].GetString();
            worker_threads = doc["worker_threads"].GetInt();
            address = doc["address"].GetString();
            port = doc["port"].GetInt();
            tcp_no_delay = doc["tcp_no_delay"].GetBool();
            use_ssl = doc["use_ssl"].GetBool();
            ssl_cert_file = doc["ssl_cert_file"].GetString();
            ssl_key_file = doc["ssl_key_file"].GetString();
            document_root = doc["document_root"].GetString();
            resource_location = doc["resource_location"].GetString();
            file_upload_location = doc["file_upload_location"].GetString();
            default_page = doc["default_page"].GetString();
            api_route = doc["api_route"].GetString();
            log_location = doc["log_location"].GetString();
            db_conn_string = doc["db_conn_string"].GetString();
            enable_cors = doc["enable_cors"].GetBool();

            for (Value::ConstMemberIterator it = doc["cors_headers"].MemberBegin();
                 it != doc["cors_headers"].MemberEnd(); ++it) {
                cors_headers.emplace(std::pair<string, string>(it->name.GetString(), it->value.GetString()));
            }

            for (Value::ConstMemberIterator it = doc["app_settings"].MemberBegin();
                 it != doc["app_settings"].MemberEnd(); ++it) {
                app_settings.emplace(std::pair<string, string>(it->name.GetString(), it->value.GetString()));
            }


            const Value& req_filters = doc["request_filters"];
            assert(req_filters.IsArray());
            for (SizeType i = 0; i < req_filters.Size(); i++) {
                request_filters.push_back(req_filters[i].GetString());
            }

            const Value& res_filters = doc["response_filters"];
            assert(res_filters.IsArray());
            for (SizeType i = 0; i < res_filters.Size(); i++) {
                response_filters.push_back(res_filters[i].GetString());
            }
    }


    string get_service_name() {
        return service_name;
    }
    int get_worker_threads() {
        return worker_threads;
    }
    string get_address() {
        return address;
    }
    int get_port() {
        return port;
    }
    string get_filters() {
        return filters;
    }
    bool get_tcp_no_delay() {
        return tcp_no_delay;
    }
    bool get_use_ssl() {
        return use_ssl;
    }
    string get_ssl_cert_file() {
        return ssl_cert_file;
    }
    string get_ssl_key_file() {
        return ssl_key_file;
    }
    string get_document_root() {
        return document_root;
    }
    string get_resource_location() {
        return resource_location;
    }
    string get_file_upload_location() {
        return file_upload_location;
    }
    string get_default_page() {
        return default_page;
    }
    string get_log_location() {
        return log_location;
    }
    string get_db_conn_string() {
        return db_conn_string;
    }
    string& get_api_route() {
        return api_route;
    }
    bool get_enable_cors() {
        return enable_cors;
    }
    unordered_map<string, string> get_app_settings() {
        return app_settings;
    }
    unordered_map<string, string> get_cors_headers() {
        return cors_headers;
    }
    vector<string> get_request_filters() {
        return request_filters;
    }
    vector<string> get_response_filters() {
        return response_filters;
    }
    vector<string> get_cors_allowed_origins() {
        return cors_allowed_origins;
    }
    vector<string> get_cors_allowed_methods() {
        return cors_allowed_methods;
    }

};