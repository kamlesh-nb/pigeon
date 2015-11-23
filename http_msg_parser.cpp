//
// Created by kamlesh on 23/11/15.
//

#include <string.h>
#include <malloc.h>
#include "http_msg_parser.h"

using namespace pigeon::tcp;

http_msg_parser::http_msg_parser() {

}

http_msg_parser::~http_msg_parser() {

}

void http_msg_parser::init() {
    parser_settings.on_url = [](http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            iConn->context->request->url += data;
            free(data);
        }
        return 0;

    };

    parser_settings.on_header_field = [](http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);

            iConn->context->request->set_header_field(s);
        }
        return 0;

    };

    parser_settings.on_header_value = [](http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);
            iConn->context->request->set_header_value(s);
        }
        return 0;

    };

    parser_settings.on_headers_complete = [](http_parser* parser) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        iConn->context->request->method = parser->method;
        iConn->context->request->http_major_version = parser->http_major;
        iConn->context->request->http_minor_version = parser->http_minor;
        return 0;

    };

    parser_settings.on_body = [](http_parser* parser, const char* at, size_t len) -> int {

        iconnection_t* iConn = (iconnection_t*)parser->data;
        if (at && iConn->context->request) {

            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            iConn->context->request->content += data;
            free(data);

        }
        return 0;

    };

    parser_settings.on_message_complete = [](http_parser* parser) -> int {


        if (r != 0){
            logger::get()->write(LogType::Error, Severity::Critical, uv_err_name(r));
        }


        return 0;

    };
}

void http_msg_parser::parse_request() {

}
