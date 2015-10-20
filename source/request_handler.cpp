#include <ios>
#include <iostream>
#include <sstream>
#include "file_info.h"
#include "file_cache.h"
#include "config.h"
#include "api_handler_base.h"
#include "request_handler.h"
#include "app_constants.h"
#include "logger.h"

using namespace pigeon;

request_handler::request_handler()
{
	resource_location = config::get()->get_resource_location();
	default_page = config::get()->get_default_page();
}

request_handler::~request_handler()
{
}

bool request_handler::url_decode(const string &in, string &out) {

	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%')
		{
			if (i + 3 <= in.size())
			{
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value)
				{
					out += static_cast<char>(value);
					i += 2;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (in[i] == '+')
		{
			out += ' ';
		}
		else
		{
			out += in[i];
		}
	}
	return true;

}

void request_handler::process_resource_request(http_request & req, http_response & res){

	try {

		std::string request_path;

		if (!url_decode(req.url, request_path))
		{
			prepare(HttpStatus::NotFound, req, res);
			return;
		}

		if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos)
		{
			prepare(HttpStatus::NotFound, req, res);
			return;
		}

		if (request_path[request_path.size() - 1] == '/')
		{
			request_path += default_page;
		}

		std::string full_path = resource_location + request_path;

		file_info fi(full_path);
		file_cache::get()->get_item(full_path, fi);

		if (fi.file_size == 0){
			prepare(HttpStatus::NotFound, req, res);
			return;
		}

		///check if the file is modified, if not send status 304
		string key = "If-Modified-Since";
		key_value_pair kvp_if_modified_since;
		kvp_if_modified_since.key = key;
		req.get_header(kvp_if_modified_since);

		if (kvp_if_modified_since.value.size() > 0){
			if (kvp_if_modified_since.value == fi.last_write_time){
				prepare(HttpStatus::NotModified, req, res);
				return;
			}
		}

		if (fi.file_size == 0){
			prepare(HttpStatus::NotFound, req, res);
			return;
		}

		prepare(HttpStatus::OK, req, res);

		///check if http compression is enabled
		string key2("Accept-Encoding");
		key_value_pair kvp_accept_enc;
		kvp_accept_enc.key = key2;
		req.get_header(kvp_accept_enc);

		std::size_t pos;

		if (kvp_accept_enc.value.size() > 0) {
			pos = kvp_accept_enc.value.find("gzip");
		}
		else
		{
			pos = string::npos;
		}

		if (pos != string::npos){
			res.message += fi.compresses_cached_headers;
			res.content += fi.compressed_content;
		}
		else {
			res.message += fi.cached_headers;
			res.content += fi.content;
		}

		finish(HttpStatus::OK, req, res);

	}
	catch (std::exception& ex){

		logger::get()->write(LogType::Error, Severity::Critical, ex.what());

	}

}

void request_handler::process_api_request(http_request & req, http_response & res){
	auto o_api_handler = config::get()->get_handler(req.url);
	if (!o_api_handler)
	{
		prepare(HttpStatus::NotFound,req, res);
		return;
	}
	o_api_handler->process(req, res);

	prepare(HttpStatus::OK, req, res);
	{
		res.message += app_constants::get_header_field(HttpHeader::Content_Length);
		res.message += std::to_string(res.content.size());
		res.message += "\r\n";

	}

	finish(HttpStatus::OK, req, res);
}

void request_handler::prepare(HttpStatus status, http_request & req, http_response & res){

	res.message += app_constants::get_status_phrase(status);
	res.message += app_constants::get_cached_response(req.is_api);

}

void request_handler::finish(HttpStatus status, http_request & req, http_response & res){

	if (status != HttpStatus::OK && status != HttpStatus::NotModified) {
		res.message += "\r\n";
		res.message += err_msg1;
		res.message += std::to_string((int)status);
		res.message += err_msg3;
		res.message += app_constants::get_status_msg(status);
		res.message += err_msg5;
	}
	else {
		res.get_non_default_headers(res.message);
		res.message += "\r\n";
		res.message += res.content;
	}

}

void request_handler::handle_request(http_request & req, http_response & res){
	
	if (req.is_api){
		process_api_request(req, res);
	}
	else {
		process_resource_request(req, res);
	}

}