//
// Created by kamlesh on 9/4/16.
//

#ifndef PIGEON_APP_CONTEXT_H
#define PIGEON_APP_CONTEXT_H

#include <memory>
#include <mutex>
#include <uv.h>
#include "http_filter_base.h"
#include "http_handler_base.h"

using namespace std;

namespace pigeon {


   class app_context {
   private:
       class settings;
       class logger;
       class cache;


       typedef shared_ptr<http_filter_base>(*CreateFilter)();
       typedef unordered_map<std::string, CreateFilter> filters;
       filters filter_registry;

       typedef shared_ptr<http_handler_base>(*CreateHandler)();
       typedef unordered_map<std::string, CreateHandler> handlers;
       handlers handler_registry;

       app_context();



       settings* _SettingsImpl;
       logger* _LoggerImpl;
       cache* _CacheImpl;

       static std::mutex _mtx;
       static std::shared_ptr<app_context> temp;

   public:
       ~app_context();
       app_context(const app_context &appContext);
       app_context& operator = (app_context const &);
       static string app_log_location;
       void load_settings();
       void load_cache();

       void add_handler(const string&, CreateHandler);
       std::shared_ptr<http_handler_base> create_handler(const std::string &) const;

       void add_filter(const string&, CreateFilter);
       std::shared_ptr<http_filter_base> create_filter(const std::string &) const;

       string get_service_name();
       int get_worker_threads();
       string get_address();
       int get_port();
       string get_filters();
       bool get_tcp_no_delay();
       bool get_use_ssl();
       string get_ssl_cert_file();
       string get_ssl_key_file();
       string get_document_root();
       string get_resource_location();
       string get_file_upload_location();
       string get_default_page();
       string get_log_location();
       string get_db_conn_string();
       string& get_api_route();
       bool get_enable_cors();
       unordered_map<string, string> get_app_settings();
       unordered_map<string, string> get_cors_headers();
       vector<string> get_request_filters();
       vector<string> get_response_filters();
       vector<string> get_cors_allowed_origins();
       vector<string> get_cors_allowed_methods();



       void get_item(file_info &fi);

       void write(LogType type, Severity severity, string message);
       void write(LogType type, string message);
       void close();

       static std::shared_ptr<app_context> &get();

   };

}



#endif //PIGEON_APP_CONTEXT_H
