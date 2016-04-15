#include <http_msg.h>
#include <http_util.h>
#include <http_parser.h>
#include <cstring>

using namespace std;
using namespace pigeon;


const char* nl = "\r\n";
const char* e = nl + strlen(nl);


http_msg::~http_msg() {

}

auto http_msg::has_cookies() -> bool {
    if (cookies.size() > 0) { return true; }
    else { return false; }
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

        content.insert(content.end(), header.first.begin(), header.first.end());
        content.insert(content.end(), header.second.begin(), header.second.end());
        content.insert(content.end(), nl, e);

    }
}

auto http_request::get_parameter(string &_key) -> string& {

    return parameters[_key];

}

auto http_request::set_parameter(string &key, string &value) -> void {
    parameters.emplace(std::pair<string, string>(key, value));
}

auto http_request::create_response(const char *msg, http_response &response, HttpStatus status) -> void {

     get_status_phrase(status, response.content);
     get_err_msg(msg, status, response.content);

}

auto http_request::create_response(string &message, http_response &response, HttpStatus status) -> void {

    response.status = (unsigned int) status;

    get_status_phrase(status, response.content);
    get_cached_response(is_api, response.content);

    if (is_api && (method != HTTP_OPTIONS)) {

        get_header_field(HttpHeader::Content_Length, response.content);
        string sz = std::to_string(message.size());
        response.content.insert(response.content.end(), sz.begin(), sz.end());

        
        response.content.insert(response.content.end(), nl, e);

    }

    response.get_non_default_headers();
     
    response.content.insert(response.content.end(), nl, e);

    response.content.insert(response.content.end(), message.begin(), message.end());

}

auto http_request::create_response(string &cached_headers, string &message, http_response &response, HttpStatus status) -> void {

    response.status = (unsigned int) status;

    get_status_phrase(status, response.content);
    get_cached_response(is_api, response.content);

    if (is_api) {

        get_header_field(HttpHeader::Content_Length, response.content);
        string sz = std::to_string(message.size());
        response.content.insert(response.content.end(), sz.begin(), sz.end());
        response.content.insert(response.content.end(), nl, e);
    }

    response.content.insert(response.content.end(), cached_headers.begin(), cached_headers.end());

    response.get_non_default_headers();

    response.content.insert(response.content.end(), nl, e);

    response.content.insert(response.content.end(), message.begin(), message.end());
    
}

http_response::~http_response() {

}

http_request::~http_request() {

}