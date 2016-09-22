#include "HttpMessage.h"
#include "http_parser.h"

using namespace pigeon::net;

const char* nl = "\r\n";
const char* e = nl + strlen(nl);

HttpMessage::~HttpMessage() {

}

auto HttpMessage::HasCookies() -> bool {
    return cookies.size() > 0;
}


auto HttpMessage::AcceptsDeflated() -> bool {

    ///check if http compression is accepted
    string accept_enc = GetHeader("Accept-Encoding");
    std::size_t pos = string::npos;
    if (accept_enc.size() > 0) {
        pos = accept_enc.find("gzip");
    }
    return pos != string::npos;

}

auto HttpMessage::SetHeader(string &key, string &value) -> void {

    headers.emplace(std::pair<string, string>(key, value));

}

auto HttpMessage::GetHeader(string key) -> string& {

    return headers[key];

}

auto HttpMessage::SetCookie(string &key, string &value) -> void {

    cookies.emplace(std::pair<string, string>(key, value));

}

auto HttpRequest::GetCookie(string key) -> string& {

    return cookies[key];

}

auto HttpMessage::GetNonDefaultHeaders() -> void {

    for (auto &header : headers) {

        buffer->Append((char *) header.first.c_str());
        buffer->Append((char *) header.second.c_str());
        buffer->Append((char *) nl);

    }
}

auto HttpRequest::GetParameter(string &_key) -> string& {

    return parameters[_key];

}

auto HttpRequest::SetParameter(string &key, string &value) -> void {
    parameters.emplace(std::pair<string, string>(key, value));
}

auto HttpRequest::CreateResponse(const char *msg, HttpResponse *response, HttpStatus status) -> void {
    response->buffer = new StringBuilder;
    GetStatusPhrase(status, response->buffer);
    GetErrorMessage(msg, is_api, status, response->buffer);
}

auto HttpRequest::CreateResponse(string &message, HttpResponse *response, HttpStatus status, bool deflate) -> void {

    string compressed_msg;
    bool accept_deflated = AcceptsDeflated();

    response->buffer = new StringBuilder;

    GetStatusPhrase(status, response->buffer);
    GetCachedResponse(is_api, response->buffer);

    if (is_api && (method != HTTP_OPTIONS)) {

        char* hdrFld = (char*) GetHeaderField(HttpHeader::Content_Length);
        response->buffer->Append(hdrFld);
        string sz; char* size;

        if(deflate && accept_deflated){
            sz = std::to_string(DeflateString(message, compressed_msg));
            size = (char*)sz.c_str();
            response->buffer->Append(size);
            response->buffer->Append((char *) nl);
            response->buffer->Append((char *) GetHeaderField(HttpHeader::Content_Encoding));
        } else {
            sz = std::to_string(message.size());
            size = (char*)sz.c_str();
            response->buffer->Append(size);
            response->buffer->Append((char *) nl);
        }
    }

    response->GetNonDefaultHeaders();
    response->buffer->Append((char *) nl);

    if(deflate && accept_deflated){
        response->buffer->Append((char *) compressed_msg.c_str(), compressed_msg.size());
    } else {
        response->buffer->Append((char *) message.c_str(), message.size());
    }


}

auto HttpRequest::CreateResponse(FileInfo &fi, HttpResponse *response, HttpStatus status) -> void {

    response->buffer = new StringBuilder;

    GetStatusPhrase(status, response->buffer);
    GetCachedResponse(is_api, response->buffer);
    if(AcceptsDeflated()){
        response->buffer->Append((char *) fi.compresses_cached_headers.c_str(), fi.compresses_cached_headers.size());
        response->GetNonDefaultHeaders();
        response->buffer->Append((char *) nl);
        response->buffer->Append((char *) fi.compressed_content.c_str(), fi.compressed_content.size());
    } else {
        response->buffer->Append((char *) fi.cached_headers.c_str(), fi.cached_headers.size());
        response->GetNonDefaultHeaders();
        response->buffer->Append((char *) nl);
        response->buffer->Append((char *) fi.content.c_str(), fi.content.size());
    }

    
}

HttpResponse::~HttpResponse() {

}

HttpRequest::~HttpRequest() {

}