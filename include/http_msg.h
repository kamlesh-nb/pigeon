#ifndef HTTP_MSG_H
#define HTTP_MSG_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <key_value_pair.h>


using namespace std;


namespace  pigeon {


    class http_msg {

    private:
        vector<key_value_pair> headers;

    public:

        virtual ~http_msg();

        int http_major_version;
        int http_minor_version;
        string content;

		auto has_headers() -> bool;

        virtual auto set_header_field(string &) -> void;

        virtual auto set_header_value(string &) -> void;

        virtual auto get_header(key_value_pair &) -> void;

        virtual auto get_non_default_headers(string &) -> void;

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

        vector<key_value_pair> parameters;
		shared_ptr<http_response> response;

    public:

        virtual ~http_request();

        string url;
        unsigned int method;
        bool is_api{false};

        auto get_parameter(key_value_pair &) -> void;

        auto set_parameter(key_value_pair &) -> void;

		auto create_response(const char*, http_response&,HttpStatus status) -> void;

		auto create_response(string&, string&, http_response&, HttpStatus status) -> void;


    };

}
#endif //HTTP_MSG_H