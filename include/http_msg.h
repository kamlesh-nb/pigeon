#ifndef HTTP_MSG_H
#define HTTP_MSG_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <http_util.h>
#include <unordered_map>
#include <sstream>

using namespace std;

namespace pigeon {


    class http_msg {

    private:
        unordered_map<string, string> headers;
        string temp;

    protected:
        unordered_map<string, string> cookies;

    public:

        virtual ~http_msg();

        int http_major_version;
        int http_minor_version;
        string content;
        ostringstream buffer;

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


    class http_response {
    private:
        unordered_map<string, string> headers;
        unordered_map<string, string> cookies;

    public:
        http_response();
        ~http_response();
        unsigned int status;
        string message;
        string content;

        auto set_cookie(string &, string &) -> void;

        auto get_cookie(string) -> string&;

        auto has_cookies() -> bool;

        auto set_header(string &, string &) -> void;

        auto get_header(string) -> string&;

        auto get_non_default_headers() -> void;

    };


    class http_request {

    private:
        unordered_map<string, string> parameters;
        unordered_map<string, string> headers;
        string temp;
        unordered_map<string, string> cookies;

    public:
        http_request();
        ~http_request();

        int http_major_version;
        int http_minor_version;
        string content;
        string url;
        unsigned int method;
        bool is_api{false};
        vector<form> forms;
        
        auto get_cookie(string) -> string&;

        auto set_cookie(string &, string &) -> void;

        auto get_parameter(string &) -> string&;

        auto set_parameter(string &, string &) -> void;

        auto has_cookies() -> bool;

        auto set_header(string &, string &) -> void;

        auto get_header(string) -> string&;

        auto create_response(const char *, http_response *response, HttpStatus status) -> void;

        auto create_response(string &, http_response *response, HttpStatus status) -> void;

        auto create_response(string &, string &, http_response *response, HttpStatus status) -> void;


    };

}
#endif //HTTP_MSG_H