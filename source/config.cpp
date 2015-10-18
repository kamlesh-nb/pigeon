#include "config.h"

#include <iostream>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>
#include <fstream>

using namespace pigeon;
using namespace rapidjson;

config::config() {

}

config::config(const config &sc) {

}

config::~config()
{
}

string& config::get_address(){

	return address;
}

string& config::get_api_route(){
	return api_route;
}

string& config::get_db_conn_str(){
	return db_conn_string;
}

string& config::get_default_page(){
	return default_page;
}

string& config::get_document_root(){
	return document_root;
}

string& config::get_log_location(){
	return log_location;
}

const int config::get_max_request_size(){
	return max_req_size;
}

int config::get_num_worker_threads(){
	return num_worker_threads;
}

string& config::get_resource_location(){
	return resource_location;
}

string& config::get_service_name(){
	return service_name;
}

string& config::get_ssl_cert_file(){
	return ssl_cert_file;
}

string& config::get_ssl_key_file(){
	return ssl_key_file;
}

bool config::get_use_ssl(){
	return use_ssl;
}

void config::add_handler(const std::string& handler_name, CreateHandler handler){
	registry[handler_name] = handler;
}

std::shared_ptr<api_handler_base> config::get_handler(const std::string& handler_name) const {

	std::shared_ptr<api_handler_base> op_handler;
	handlers::const_iterator regEntry = registry.find(handler_name);
	if (regEntry != registry.end())
	{
		op_handler = regEntry->second();
	}
	return op_handler;

}

void config::load_config(string& path){

	try{

		std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
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
		max_req_size = doc["max_request_size"].GetInt();
		document_root = doc["document_root"].GetString();
		resource_location = doc["resource_location"].GetString();
		default_page = doc["default_page"].GetString();
		api_route = doc["api_route"].GetString();
		log_location = doc["log_location"].GetString();
		db_conn_string = doc["db_conn_string"].GetString();

	}
	catch (std::exception& ex){
		cerr << ex.what() << endl;
	}

}

std::shared_ptr<config> config::instance = nullptr;

std::mutex config::_mtx;

std::shared_ptr<config>& config::get()
{
	static std::shared_ptr<config> tmp = instance;

	if (!tmp)
	{
		std::lock_guard<std::mutex> lock(_mtx);

		if (!tmp)
		{
			instance.reset(new config);
			tmp = instance;
		}
	}
	return tmp;
}

int config::get_port() {
	return port;
}
