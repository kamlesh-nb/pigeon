#include <http_msg.h>
#include <http_util.h>
#include <http_parser.h>

using namespace std;
using namespace pigeon;

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

auto http_msg::get_non_default_headers(string &msg) -> void {


    for (auto &header : headers) {
        msg += header.first;
        msg += header.second;
        msg += "\r\n";
    }
}

auto http_request::get_parameter(string &_key) -> string& {

    return parameters[_key];

}

auto http_request::set_parameter(string &key, string &value) -> void {
    parameters.emplace(std::pair<string, string>(key, value));
}

auto http_request::create_response(const char *msg, http_response &response, HttpStatus status) -> void {

    response.message += get_status_phrase(status);
    response.message += get_err_msg(msg, status);;


}

auto http_request::create_response(string &message, http_response &response, HttpStatus status) -> void {

    response.content += message;
    response.status = (unsigned int) status;

    response.message += get_status_phrase(status);
    response.message += get_cached_response(is_api);

    if (is_api && (method != HTTP_OPTIONS)) {

        response.message += get_header_field(HttpHeader::Content_Length);
        response.message += std::to_string(message.size());
        response.message += "\r\n";

    }

    response.get_non_default_headers(response.message);
    response.message += "\r\n";
    response.message += response.content;

}

auto http_request::create_response(string &cached_headers, string &message, http_response &response, HttpStatus status) -> void {

    response.content += message;
    response.status = (unsigned int) status;

    response.message += get_status_phrase(status);
    response.message += get_cached_response(is_api);

    if (is_api) {

        response.message += get_header_field(HttpHeader::Content_Length);
        response.message += std::to_string(message.size());
        response.message += "\r\n";

    }

    response.message += cached_headers;
    response.get_non_default_headers(response.message);
    response.message += "\r\n";
    response.message += response.content;

}

http_response::~http_response() {

}

http_request::~http_request() {

}