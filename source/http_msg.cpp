#include <http_msg.h>
#include <http_parser.h>
#include <iostream>

using namespace std;
using namespace pigeon;


const char* nl = "\r\n";
const char* e = nl + strlen(nl);


http_msg::~http_msg() {

}

auto http_msg::has_cookies() -> bool {
    return cookies.size() > 0;
}

auto http_msg::set_header(string &key, string &value) -> void {

    headers.emplace(std::pair<string, string>(key, value));

}

auto http_msg::get_header(string key) -> string& {

    return headers[key];

}

auto http_msg::set_cookie(string &key, string &value) -> void {

    cookies.emplace(std::pair<string, string>(key, value));

}

auto http_request::get_cookie(string key) -> string& {

    return cookies[key];

}

auto http_msg::get_non_default_headers() -> void {

    for (auto &header : headers) {

        buffer->append((char*)header.first.c_str());
        buffer->append((char*)header.second.c_str());
        buffer->append((char*)nl);

    }
}

auto http_request::get_parameter(string &_key) -> string& {

    return parameters[_key];

}

auto http_request::set_parameter(string &key, string &value) -> void {
    parameters.emplace(std::pair<string, string>(key, value));
}

auto http_request::create_response(const char *msg, http_response *response, HttpStatus status) -> void {
    response->buffer = new string_builder;

    char* status_phrase = (char*)get_status_phrase(status);
    response->buffer->append(status_phrase);

    get_err_msg(msg, status, response->buffer);

}

auto http_request::create_response(string &message, http_response *response, HttpStatus status) -> void {

    response->status = (unsigned int) status;

    response->buffer = new string_builder;

    char* status_phrase = (char*)get_status_phrase(status);
    response->buffer->append(status_phrase);
    get_cached_response(is_api, response->buffer);

    if (is_api && (method != HTTP_OPTIONS)) {

        char* hdrFld = (char*)get_header_field(HttpHeader::Content_Length);
        response->buffer->append(hdrFld);
        string sz = std::to_string(message.size());
        char* size = (char*)sz.c_str();
        response->buffer->append(size);
        response->buffer->append((char*)nl);
    }

    response->get_non_default_headers();
    response->buffer->append((char*)nl);
    response->buffer->append((char*)message.c_str(), message.size());

}

auto http_request::create_response(string &cached_headers, string &message, http_response *response, HttpStatus status) -> void {

    response->status = (unsigned int) status;
    response->buffer = new string_builder;

    char* status_phrase = (char*)get_status_phrase(status);
    response->buffer->append(status_phrase);
    get_cached_response(is_api, response->buffer);


    response->buffer->append((char*)cached_headers.c_str(), cached_headers.size());
    response->get_non_default_headers();
    response->buffer->append((char*)nl);
    response->buffer->append((char*)message.c_str(), message.size());
    
}

http_response::~http_response() {

}

http_request::~http_request() {

}