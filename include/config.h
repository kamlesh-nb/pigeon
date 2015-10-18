#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <memory>
#include <mutex>
#include <map>

#include "app_constants.h"
#include "api_handler_base.h"

using namespace std;

namespace pigeon {

	class config {
	private:


		string service_name;
		string address;
		int port;
		bool use_ssl;
		string ssl_cert_file;
		string ssl_key_file;
		int num_worker_threads;
		int max_req_size;
		string document_root;
		string resource_location;
		string default_page;
		string log_location;
		string db_conn_string;
		string api_route;

		typedef std::shared_ptr<api_handler_base>(*CreateHandler)();

		typedef map<std::string, CreateHandler> handlers;
		handlers registry;

		config();

		config(const config &sc);

		static std::mutex _mtx;

		static std::shared_ptr<config> instance;

	public:

		static std::shared_ptr<config> &get();

		~config();

		string &get_service_name();

		string &get_address();

		int get_port();

		bool get_use_ssl();

		string &get_ssl_cert_file();

		string &get_ssl_key_file();

		int get_num_worker_threads();

		const int get_max_request_size();

		string &get_document_root();

		string &get_resource_location();

		string &get_default_page();

		string &get_api_route();

		string &get_log_location();

		string &get_db_conn_str();

		void add_handler(const std::string &, CreateHandler);

		std::shared_ptr<api_handler_base> get_handler(const std::string &) const;

		void load_config(string &);

	};
}

#endif //CONFIG_H
