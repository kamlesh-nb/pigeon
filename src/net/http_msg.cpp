#include "http_msg.h"
#include "http_parser.h"

using namespace pigeon::net;

const char* nl = "\r\n";
const char* e = nl + strlen(nl);

http_msg::~http_msg() {

}

auto http_msg::has_cookies() -> bool {
    return cookies.size() > 0;
}


auto http_msg::accepts_deflated() -> bool {

    ///check if http compression is accepted
    string accept_enc = get_header("Accept-Encoding");
    std::size_t pos = string::npos;
    if (accept_enc.size() > 0) {
        pos = accept_enc.find("gzip");
    }
    return pos != string::npos;

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
    get_status_phrase(status, response->buffer);
    get_err_msg(msg, is_api , status, response->buffer);
}

auto http_request::create_response(string &message, http_response *response, HttpStatus status, bool deflate) -> void {

    string compressed_msg;
    bool accept_deflated = accepts_deflated();

    response->buffer = new string_builder;

    get_status_phrase(status, response->buffer);
    get_cached_response(is_api, response->buffer);

    if (is_api && (method != HTTP_OPTIONS)) {

        char* hdrFld = (char*)get_header_field(HttpHeader::Content_Length);
        response->buffer->append(hdrFld);
        string sz; char* size;

        if(deflate && accept_deflated){
            sz = std::to_string(deflate_string(message, compressed_msg));
            size = (char*)sz.c_str();
            response->buffer->append(size);
            response->buffer->append((char*)nl);
            response->buffer->append((char*)get_header_field(HttpHeader::Content_Encoding));
        } else {
            sz = std::to_string(message.size());
            size = (char*)sz.c_str();
            response->buffer->append(size);
            response->buffer->append((char*)nl);
        }
    }

    response->get_non_default_headers();
    response->buffer->append((char*)nl);

    if(deflate && accept_deflated){
        response->buffer->append((char*)compressed_msg.c_str(), compressed_msg.size());
    } else {
        response->buffer->append((char*)message.c_str(), message.size());
    }


}

auto http_request::create_response(file_info& fi, http_response *response, HttpStatus status) -> void {

    response->buffer = new string_builder;

    get_status_phrase(status, response->buffer);
    get_cached_response(is_api, response->buffer);
    if(accepts_deflated()){
        response->buffer->append((char*)fi.compresses_cached_headers.c_str(), fi.compresses_cached_headers.size());
        response->get_non_default_headers();
        response->buffer->append((char*)nl);
        response->buffer->append((char*)fi.compressed_content.c_str(), fi.compressed_content.size());
    } else {
        response->buffer->append((char*)fi.cached_headers.c_str(), fi.cached_headers.size());
        response->get_non_default_headers();
        response->buffer->append((char*)nl);
        response->buffer->append((char*)fi.content.c_str(), fi.content.size());
    }

    
}

http_response::~http_response() {

}

http_request::~http_request() {

}