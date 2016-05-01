#include <http_msg.h>
#include <http_util.h>
#include <http_parser.h>
#include <cstring>
#include <sstream>

using namespace std;
using namespace pigeon;


const char* nl = "\r\n";
const char* e = nl + strlen(nl);


 

auto http_response::has_cookies() -> bool {
    if (cookies.size() > 0) { return true; }
    else { return false; }
}

auto http_response::set_header(string &key, string &value) -> void {

    headers.emplace(std::pair<string, string>(key, value));

}

auto http_response::get_header(string key) -> string& {

    return headers[key];

}

auto http_response::set_cookie(string &key, string &value) -> void {

    cookies.emplace(std::pair<string, string>(key, value));

}


auto http_response::get_cookie(string key) -> string& {

    return cookies[key];

}

auto http_request::has_cookies() -> bool {
    if (cookies.size() > 0) { return true; }
    else { return false; }
}

auto http_request::set_header(string &key, string &value) -> void {

    headers.emplace(std::pair<string, string>(key, value));

}

auto http_request::get_header(string key) -> string& {

    return headers[key];

}

auto http_request::set_cookie(string &key, string &value) -> void {

    cookies.emplace(std::pair<string, string>(key, value));

}

auto http_request::get_cookie(string key) -> string& {

    return cookies[key];

}

auto http_response::get_non_default_headers() -> void {

    for (auto &header : headers) {

        content.append(header.first);
        content.append(header.second);
        content.append(nl);

    }
}

auto http_request::get_parameter(string &_key) -> string& {

    return parameters[_key];

}

auto http_request::set_parameter(string &key, string &value) -> void {
    parameters.emplace(std::pair<string, string>(key, value));
}

auto http_request::create_response(const char *msg, http_response *response, HttpStatus status) -> void {
    response->message.append(get_status_phrase(status));
    response->message.append(get_err_msg(msg, status));

}

auto http_request::create_response(string &message, http_response *response, HttpStatus status) -> void {

    response->status = (unsigned int) status;
    response->message.append(get_status_phrase(status));
    response->message.append(get_cached_response(is_api));

    if (is_api && (method != HTTP_OPTIONS)) {

        response->message.append(get_header_field(HttpHeader::Content_Length));
        string sz = std::to_string(message.size());
        response->message.append(sz);
        response->message.append(nl);
    }

    response->get_non_default_headers();
    response->message.append(content);
    response->message.append(nl);
    response->message.append(message);

}

auto http_request::create_response(string &cached_headers, string &message, http_response *response, HttpStatus status) -> void {

    std::ostringstream oss;

    response->status = (unsigned int) status;
    response->message.append(get_status_phrase(status));
    response->message.append(get_cached_response(is_api));

    if (is_api) {

        response->message.append(get_header_field(HttpHeader::Content_Length));
        string sz = std::to_string(message.size());
        response->message.append(sz);
        response->message.append(nl);
    }

    response->message.append(cached_headers);
    response->get_non_default_headers();
    response->message.append(content);
    response->message.append(nl);
    response->message.append(message);
    
}


http_response::~http_response() {

}

http_request::~http_request() {

}


http_response::http_response() {

}

http_request::http_request() {

}