//
// Created by kamlesh on 23/11/15.
//

#include <string.h>
#include <malloc.h>
#include "http_msg.h"
#include "http_msg_parser.h"

using namespace pigeon;
using namespace pigeon::tcp;

http_msg_parser::http_msg_parser() {

}

http_msg_parser::~http_msg_parser() {

}

void http_msg_parser::init() {

    parser = (http_parser*)malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    std::memset(&parser_settings, 0, sizeof(parser_settings));
    
    parser_settings.on_url = [](http_parser* parser, const char* at, size_t len) -> int {

        http_request* req = (http_request*)parser->data;
        if (at && req) {
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            req->url += data;
            free(data);
        }
        return 0;

    };

    parser_settings.on_header_field = [](http_parser* parser, const char* at, size_t len) -> int {

         http_request* req = (http_request*)parser->data;
        if (at && req) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);

            req->set_header_field(s);
        }
        return 0;

    };

    parser_settings.on_header_value = [](http_parser* parser, const char* at, size_t len) -> int {

         http_request* req = (http_request*)parser->data;
        if (at && req) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);
            req->set_header_value(s);
        }
        return 0;

    };

    parser_settings.on_headers_complete = [](http_parser* parser) -> int {

         http_request* req = (http_request*)parser->data;
        req->method = parser->method;
        req->http_major_version = parser->http_major;
        req->http_minor_version = parser->http_minor;
        return 0;

    };

    parser_settings.on_body = [](http_parser* parser, const char* at, size_t len) -> int {

        http_request* req = (http_request*)parser->data;
        if (at && req) {

            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            req->content += data;
            free(data);

        }
        return 0;

    };

    parser_settings.on_message_complete = [](http_parser* parser) -> int {


     

        return 0;

    };
}

void http_msg_parser::parse_request() {

}
