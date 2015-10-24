//
// Created by kamlesh on 23/10/15.
//

#include <uv.h>
#include "settings.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>
#include <fstream>

using namespace rapidjson;

using namespace pigeon;

auto settings::get_address() -> string& {

    return address;
}

auto settings::get_port() -> int {
    return port;
}

auto settings::get_api_route() -> string& {
    return api_route;
}

auto settings::get_db_conn_str() -> string& {
    return db_conn_string;
}

auto settings::get_default_page() -> string& {
    return default_page;
}

auto settings::get_document_root() -> string& {
    return document_root;
}

auto settings::get_log_location() -> string& {
    return log_location;
}

auto settings::get_num_worker_threads() -> int {
    return num_worker_threads;
}

auto settings::get_resource_location() -> string& {
    return resource_location;
}

auto settings::get_service_name() -> string& {
    return service_name;
}

auto settings::get_ssl_cert_file() -> string& {
    return ssl_cert_file;
}

auto settings::get_ssl_key_file() -> string& {
    return ssl_key_file;
}

auto settings::get_use_ssl() -> bool{
    return use_ssl;
}

auto settings::set_app_setting_key(string& _key) -> void{

    key_value_pair kvp;
    kvp.key = _key;
    appsettings.push_back(kvp);

}

auto settings::set_app_setting_value(string& _value) -> void{

    if(appsettings.empty()) {
        return;
    }
    appsettings.back().value = _value;

}

auto settings::get_app_setting(key_value_pair& kvp) -> void {

    for(auto& hdr : appsettings){
        if(hdr.key == kvp.key){
            kvp = hdr;
        }
    }

}

auto settings::load_setting() -> void {

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
    address = doc["address"].GetString();
    port = doc["port"].GetInt();
    use_ssl = doc["use_ssl"].GetBool();
    ssl_cert_file = doc["ssl_cert_file"].GetString();
    ssl_key_file = doc["ssl_key_file"].GetString();
    num_worker_threads = doc["worker_threads"].GetInt();
    document_root = doc["document_root"].GetString();
    resource_location = doc["resource_location"].GetString();
    default_page = doc["default_page"].GetString();
    api_route = doc["api_route"].GetString();
    log_location = doc["log_location"].GetString();
    db_conn_string = doc["db_conn_string"].GetString();

}
