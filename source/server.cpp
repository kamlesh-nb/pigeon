//
// Created by kamlesh on 17/10/15.
//

#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <assert.h>
#include "http_handler.h"
#include "cache.h"
#include <http_parser.h>
#include "server.h"
#include "http_context.h"
#include <uv_http.h>

#define VARNAME TEXT("UV_THREADPOOL_SIZE")
#define BUFSIZE 4096
#define BUFFER_SIZE 255

#define MAX_WRITE_HANDLES 1000


 
 
namespace pigeon {


	server::server() { }

	server::~server() { }

	void server::_init(){

		http::_Settings = new settings;
		http::_Cache = new cache;
		http::_Settings->load_setting();

		//initialise loop, so that we can add filie watchers when loading them into the cache
		http::uv_loop = uv_default_loop();
		http::_Cache->load(http::_Settings->get_resource_location());

	}

	void server::_tcp(){

		int r = uv_tcp_init(http::uv_loop, &http::uv_tcp);

		if (r != 0){
			logger::get(http::_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
		}

		r = uv_tcp_keepalive(&http::uv_tcp, 1, 60);
		if (r != 0){
			logger::get(http::_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
		}

	}

	void server::_bind(){

		string _address = http::_Settings->get_address();
		int _port = http::_Settings->get_port();

		struct sockaddr_in address;
		int r = uv_ip4_addr(_address.c_str(), _port, &address);
		if (r != 0){
			logger::get(http::_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
		}

		r = uv_tcp_bind(&http::uv_tcp, (const struct sockaddr*)&address, 0);
		if (r != 0){
			logger::get(http::_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
		}

	}

	void server::_listen(){

		int r = uv_listen((uv_stream_t*)&http::uv_tcp, MAX_WRITE_HANDLES, http::server::on_connect);

		if (r != 0){
			logger::get(http::_Settings)->write(LogType::Error, Severity::Critical, uv_err_name(r));
		}

		logger::get(http::_Settings)->write(LogType::Information, Severity::Medium, "Server Started...");

		uv_run(http::uv_loop, UV_RUN_DEFAULT);

	}

	void server::start() {
 
#ifndef _WIN32
		signal(SIGPIPE, SIG_IGN);
#endif
		_init();
		_pool_size();
		_parser();
		_tcp();
		_bind();
		_listen();

	}

	void server::stop() {

		uv_stop(http::uv_loop);

	}

	void server::_pool_size() {

#ifdef _WIN32
		string num_of_threads = std::to_string(http::_Settings->get_num_worker_threads());
		SetEnvironmentVariable(VARNAME, (LPTSTR)num_of_threads.c_str());
#else
		stringstream ss;
		ss << m_settings->get_num_worker_threads();
		setenv("UV_THREADPOOL_SIZE", ss.str().c_str(), 1);
#endif


	}
 
	void server::_parser() {

		http::parser_settings.on_url = http::parser::on_url;
		http::parser_settings.on_header_field = http::parser::on_header_field;
		http::parser_settings.on_header_value = http::parser::on_header_value;
		http::parser_settings.on_headers_complete = http::parser::on_headers_complete;
		http::parser_settings.on_body = http::parser::on_body;
		http::parser_settings.on_message_complete = http::parser::on_message_complete;

	}

}