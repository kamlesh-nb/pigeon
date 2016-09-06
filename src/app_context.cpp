//
// Created by kamlesh on 9/5/16.
//

#include "app_context.h"
#include "settings.ipp"
#include "cache.ipp"
#include "logger.ipp"

using namespace pigeon;

std::mutex app_context::_mtx;
std::shared_ptr<app_context> app_context::temp = nullptr;
string app_context::app_log_location;

app_context::app_context() {


}

app_context::~app_context() {

}

app_context::app_context(const app_context &appContext) {
    temp = appContext.temp;
}

app_context &app_context::operator=(const app_context& appContext ) {

    if (this != &appContext) {
        temp = appContext.temp;
    }
    return *this;
}


void app_context::load_settings() {

    _SettingsImpl = new settings;
    _LoggerImpl = new logger;
    _CacheImpl = new cache;

    _SettingsImpl->load_settings();
    app_log_location = _SettingsImpl->get_log_location();
}

void app_context::load_cache() {
    _CacheImpl->load(_SettingsImpl->get_resource_location());
}


void app_context::add_handler(const string& handler_name, app_context::CreateHandler func) {
    handler_registry[handler_name] = func;
}

std::shared_ptr<http_handler_base> app_context::create_handler(const std::string& handler_name) const {
    std::shared_ptr<http_handler_base> _handler;
    handlers::const_iterator regEntry = handler_registry.find(handler_name);
    if (regEntry != handler_registry.end())
    {
        _handler = regEntry->second();
    }
    return _handler;
}

void app_context::add_filter(const string& filter_name, app_context::CreateFilter func) {
    filter_registry[filter_name] = func;
}

std::shared_ptr<http_filter_base> app_context::create_filter(const std::string& filter_name) const {
    std::shared_ptr<http_filter_base> _filter;
    filters::const_iterator regEntry = filter_registry.find(filter_name);
    if (regEntry != filter_registry.end())
    {
        _filter = regEntry->second();
    }
    return _filter;
}


string app_context::get_service_name() {
    return _SettingsImpl->get_service_name();
}
int app_context::get_worker_threads() {
    return _SettingsImpl->get_worker_threads();
}
string app_context::get_address() {
    return _SettingsImpl->get_address();
}
int app_context::get_port() {
    return _SettingsImpl->get_port();
}
string app_context::get_filters() {
    return _SettingsImpl->get_filters();
}
bool app_context::get_tcp_no_delay() {
    return _SettingsImpl->get_tcp_no_delay();
}
bool app_context::get_use_ssl() {
    return _SettingsImpl->get_use_ssl();
}
string app_context::get_ssl_cert_file() {
    return _SettingsImpl->get_ssl_cert_file();
}
string app_context::get_ssl_key_file() {
    return _SettingsImpl->get_ssl_key_file();
}
string app_context::get_document_root() {
    return _SettingsImpl->get_document_root();
}
string app_context::get_resource_location() {
    return _SettingsImpl->get_resource_location();
}
string app_context::get_file_upload_location() {
    return _SettingsImpl->get_file_upload_location();
}
string app_context::get_default_page() {
    return _SettingsImpl->get_default_page();
}
string app_context::get_log_location() {
    return _SettingsImpl->get_log_location();
}
string app_context::get_db_conn_string() {
    return _SettingsImpl->get_db_conn_string();
}
string& app_context::get_api_route() {
    return _SettingsImpl->get_api_route();
}
bool app_context::get_enable_cors() {
    return _SettingsImpl->get_enable_cors();
}
unordered_map<string, string> app_context::get_app_settings() {
    return _SettingsImpl->get_app_settings();
}
unordered_map<string, string> app_context::get_cors_headers() {
    return _SettingsImpl->get_cors_headers();
}
vector<string> app_context::get_request_filters() {
    return _SettingsImpl->get_request_filters();
}
vector<string> app_context::get_response_filters() {
    return _SettingsImpl->get_response_filters();
}
vector<string> app_context::get_cors_allowed_origins() {
    return _SettingsImpl->get_cors_allowed_origins();
}
vector<string> app_context::get_cors_allowed_methods() {
    return _SettingsImpl->get_cors_allowed_methods();
}


void app_context::get_item(file_info &fi){
    _CacheImpl->get_item(fi);
}


void app_context::write(LogType type, Severity severity, string message) {
  _LoggerImpl->write(type, severity, message);
}
void app_context::write(LogType type, string message) {
    _LoggerImpl->write(type, message);
}
void app_context::close() {
    _LoggerImpl->close();
}


std::shared_ptr<app_context> &app_context::get() {
    static std::shared_ptr<app_context> tmp = temp;

    if (!tmp) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!tmp) {
            temp.reset(new app_context);
            tmp = temp;
        }
    }
    return tmp;
}





