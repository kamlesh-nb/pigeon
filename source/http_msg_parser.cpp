//
// Created by kamlesh on 23/11/15.
//

#include <string.h>
#include <malloc.h>
#include <sstream>

#include <string>
#include <iterator>
#include "http_msg.h"
#include "http_msg_parser.h"
#include "http_util.h"

using namespace std;
using namespace pigeon;
using namespace pigeon::tcp;

http_msg_parser::http_msg_parser() {
    init();
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

        http_request* req = (http_request*)parser->data;




        return 0;

    };
}

bool http_msg_parser::is_api(const string &Uri) {
    std::size_t pos = Uri.find("/api/");
    return pos != string::npos;
}

void http_msg_parser::parse_query_string(http_request &req) {
    string query_uri(req.url);
    std::size_t _parStart = req.url.find('?');

    if (_parStart != string::npos) {

        req.url = query_uri.substr(0, _parStart);
        query_uri = query_uri.substr(_parStart + 1, query_uri.size());
    }

    if (_parStart != string::npos) {
        replace(query_uri.begin(), query_uri.end(), '&', ' ');
        std::istringstream issParams(query_uri.c_str());
        vector<string> vparams{istream_iterator<string>{issParams},
                               istream_iterator<string>{}};
        size_t end;
        for (auto &par : vparams) {
            end = par.find("=", 0);
            key_value_pair kvp;
            kvp.key = par.substr(0, end);
            kvp.value = par.substr(end + 1, par.size() - 1);
            req.set_parameter(kvp);
        }
    }
}

void http_msg_parser::parse_request(char* data, asio::ip::tcp::socket client, size_t nread) {

    size_t parsed;
    parser->data = new http_request;
    parsed = (size_t)http_parser_execute(parser, &parser_settings, data, nread);

}
