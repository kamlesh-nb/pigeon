#ifndef HTTP_MSG_H
#define HTTP_MSG_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <http_util.h>
#include <unordered_map>


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

        auto has_headers() -> bool;

        virtual auto set_header(string &, string &) -> void;

        virtual auto get_header(string) -> string;

        virtual auto set_cookie(string &, string &) -> void;

        virtual auto get_non_default_headers(string &) -> void;

    };

    struct form {
        unordered_map<string, string> headers;
        unordered_map<string, string> parameters;
        string file_data;
    };

    class http_response : public http_msg {


    public:
        virtual ~http_response();

    public:

        unsigned int status;
        string message;


    };

    class http_request : public http_msg {

    private:

        unordered_map<string, string> parameters;
        shared_ptr<http_response> response;

    public:

        virtual ~http_request();

        string url;
        unsigned int method;
        bool is_api{false};
        vector<form> forms;

        char *data;

        auto get_cookie(string) -> string;

        auto get_parameter(string &) -> string;

        auto set_parameter(string &, string &) -> void;

        auto create_response(const char *, http_response &, HttpStatus status) -> void;

        auto create_response(string &, http_response &, HttpStatus status) -> void;

        auto create_response(string &, string &, http_response &, HttpStatus status) -> void;


    };

}
#endif //HTTP_MSG_H