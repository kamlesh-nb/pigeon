#ifndef PIGEON_HTTP_MSG_H
#define PIGEON_HTTP_MSG_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include "http_util.h"
#include "string_builder.h"
#include "file_info.h"

using namespace std;

namespace pigeon {


    class http_msg {
    private:
        unordered_map<string, string> headers;
        string temp;
    protected:
        unordered_map<string, string> cookies;
        auto accepts_deflated() -> bool;
    public:
        virtual ~http_msg();
        int http_major_version;
        int http_minor_version;
        string content;
        string_builder* buffer;
        auto has_cookies() -> bool;

        virtual auto set_header(string &, string &) -> void;
        virtual auto get_header(string) -> string&;
        virtual auto set_cookie(string &, string &) -> void;
        virtual auto get_non_default_headers() -> void;
    };

    struct form {
        unordered_map<string, string> headers;
        unordered_map<string, string> parameters;
        string file_data;
    };

    class http_response : public http_msg {
    public:
        virtual ~http_response();
        string message;
    };


    class http_request : public http_msg {
    private:
        unordered_map<string, string> parameters;
    public:
        virtual ~http_request();
        string url;
        unsigned int method;
        bool is_api{false};
        vector<form> forms;
        auto get_cookie(string) -> string&;
        auto get_parameter(string &) -> string&;
        auto set_parameter(string &, string &) -> void;
        auto create_response(const char *, http_response *response, HttpStatus status) -> void;
        auto create_response(string &, http_response *response, HttpStatus status, bool deflate) -> void;
        auto create_response(file_info&, http_response *response, HttpStatus status) -> void;
    };

}
#endif //PIGEON_HTTP_MSG_H