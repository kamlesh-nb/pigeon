//
// Created by kamlesh on 23/10/15.
//

#include <uv.h>
#include <settings.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <fstream>
#include <logger.h>

using namespace rapidjson;
using namespace pigeon;

string settings::service_name;
int settings::worker_threads;
string settings::address;
int settings::port;
string settings::filters;
bool  settings::tcp_no_delay;
bool settings::use_ssl;
string settings::ssl_cert_file;
string settings::ssl_key_file;
string settings::document_root;
string settings::resource_location;
string settings::file_upload_location;
string settings::default_page;
string settings::log_location;
string settings::db_conn_string;
string settings::api_route;
bool settings::enable_cors;
unordered_map<string, string> settings::app_settings;
unordered_map<string, string> settings::cors_headers;
vector<string> settings::request_filters;
vector<string> settings::response_filters;
vector<string> settings::cors_allowed_origins;
vector<string> settings::cors_allowed_methods;


auto settings::load_setting() -> void {

    try {

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
    catch (std::exception &ex) {
        logger::get()->write(LogType::Error, ex.what());
    }


}



