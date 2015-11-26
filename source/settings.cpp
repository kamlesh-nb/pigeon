//
// Created by kamlesh on 23/10/15.
//

#include "settings.h"
#include <boost/filesystem.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <fstream>
#include <iostream>

namespace bfs = boost::filesystem;
using namespace rapidjson;
using namespace pigeon;

string settings::service_name;
string settings::address;
string settings::port;
int settings::worker_threads;
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


auto settings::load_setting() -> void {

    try {

        
        string current_path = bfs::current_path().string();
        current_path.append("/service.json");
		cout << current_path << endl;

        std::ifstream is(current_path.c_str(), std::ios::in | std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(is)), (std::istreambuf_iterator<char>()));

        Document doc;
        doc.Parse(content.c_str());

        service_name = doc["service_name"].GetString();
        address = doc["address"].GetString();
		port = doc["port"].GetString();
		worker_threads = doc["worker_threads"].GetInt();
        use_ssl = doc["use_ssl"].GetBool();
        ssl_cert_file = doc["ssl_cert_file"].GetString();
        ssl_key_file = doc["ssl_key_file"].GetString();
        document_root = doc["document_root"].GetString();
        resource_location = doc["resource_location"].GetString();
        default_page = doc["default_page"].GetString();
        api_route = doc["api_route"].GetString();
        log_location = doc["log_location"].GetString();
        db_conn_string = doc["db_conn_string"].GetString();
        enable_cors = doc["enable_cors"].GetBool();

        for (Value::ConstMemberIterator it = doc["cors_headers"].MemberBegin(); it != doc["cors_headers"].MemberEnd(); ++it) {
            cors_headers.emplace(std::pair<string, string>(it->name.GetString(), it->value.GetString()));
        }

        for (Value::ConstMemberIterator it = doc["app_settings"].MemberBegin(); it != doc["app_settings"].MemberEnd(); ++it) {
            app_settings.emplace(std::pair<string, string>(it->name.GetString(), it->value.GetString()));
        }
    }
    catch(std::exception& ex){
        cout << ex.what() << endl;
    }


}



