#include "http_msg.h"

using namespace pigeon;

http_msg::~http_msg() {

}

auto http_msg::set_header_field(string& _key) -> void{

    key_value_pair kvp;
    kvp.key = _key;
    headers.push_back(kvp);

}

auto http_msg::set_header_value(string& _value) -> void{

    if(headers.empty()) {
        return;
    }
    headers.back().value = _value;

}

auto http_msg::get_header(key_value_pair& kvp) -> void {

    for(auto& hdr : headers){
        if(hdr.key == kvp.key){
            kvp = hdr;
        }
    }

}

auto http_msg::get_non_default_headers(string &msg) -> void {

    for (auto& hdr : headers){
        msg += hdr.key;
        msg += hdr.value;
        msg += "\r\n";
    }

}

auto http_request::get_parameter(key_value_pair& kvp) -> void {

    for(auto& prm : parameters){
        if(prm.key == kvp.key){
            kvp = prm;
        }
    }

}

auto http_request::get_response()-> shared_ptr<http_response> {
	return response;
}

auto http_request::create_response(const char* msg, HttpStatus status) -> void {

	response = make_shared<http_response>();

	response->message += get_status_phrase(status);
	response->message += get_cached_response(is_api);
	response->message += get_header_field(HttpHeader::Content_Length);
	response->message += std::to_string(string(msg).size());
	response->message += "\r\n";
	response->message += get_err_msg(status);
 

}

auto http_request::create_response(string& cached_headers, string& message, HttpStatus status) -> void {
	
	response = make_shared<http_response>();

	response->message += get_status_phrase(status);
	response->message += get_cached_response(is_api);
	response->message += get_header_field(HttpHeader::Content_Length);
	response->message += std::to_string(message.size());
	response->message += "\r\n";
	response->content += message;
	response->status = (unsigned int)status;
	
	response->get_non_default_headers(response->message);
	response->message += "\r\n";
	response->message += cached_headers;
	response->message += response->content;

 

}

auto http_request::set_parameter(key_value_pair& kvp) -> void {
    parameters.push_back(std::move(kvp));
}


std::vector<asio::const_buffer> http_response::to_buffers() {
    std::vector<asio::const_buffer> buffers;
    buffers.push_back(asio::buffer(message));
    return buffers;
}

http_response::~http_response() {

}

http_request::~http_request() {

}