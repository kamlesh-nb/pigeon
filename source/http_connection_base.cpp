//
// Created by kamlesh on 24/11/15.
//

#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include "http_connection_base.h"

using namespace pigeon::tcp;


http_connection_base::http_connection_base(asio::io_context &io_context) : client(io_context){
    context = make_shared<http_context>();
}

asio::ip::tcp::socket&http_connection_base::socket() {
    return client;
}


void http_connection_base::init_parser() {

    parser = (http_parser*)malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    std::memset(&parser_settings, 0, sizeof(parser_settings));

    parser_settings.on_url = [](http_parser* parser, const char* at, size_t len) -> int {

        http_connection_base* con = static_cast<http_connection_base*>(parser->data);
        if (at && con->context->request) {
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            con->context->request->url += data;
            free(data);
        }
        return 0;

    };

    parser_settings.on_header_field = [](http_parser* parser, const char* at, size_t len) -> int {

        http_connection_base* con = static_cast<http_connection_base*>(parser->data);
        if (at && con->context->request) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);

            con->context->request->set_header_field(s);
        }
        return 0;

    };

    parser_settings.on_header_value = [](http_parser* parser, const char* at, size_t len) -> int {

        http_connection_base* con = static_cast<http_connection_base*>(parser->data);
        if (at && con->context->request) {
            string s;
            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            s += data;
            free(data);
            con->context->request->set_header_value(s);
        }
        return 0;

    };

    parser_settings.on_headers_complete = [](http_parser* parser) -> int {

        http_connection_base* con = static_cast<http_connection_base*>(parser->data);
        con->context->request->method = parser->method;
        con->context->request->http_major_version = parser->http_major;
        con->context->request->http_minor_version = parser->http_minor;
        return 0;

    };

    parser_settings.on_body = [](http_parser* parser, const char* at, size_t len) -> int {

        http_connection_base* con = static_cast<http_connection_base*>(parser->data);
        if (at && con->context->request) {

            char *data = (char *)malloc(sizeof(char) * len + 1);
            strncpy(data, at, len);
            data[len] = '\0';
            con->context->request->content += data;
            free(data);

        }
        return 0;

    };

    parser_settings.on_message_complete = [](http_parser* parser) -> int {

        http_connection_base* con = static_cast<http_connection_base*>(parser->data);
        con->do_write();

        return 0;

    };
}

void http_connection_base::parse_request(size_t nread) {
    size_t parsed;

    parsed = (size_t)http_parser_execute(parser, &parser_settings, buffer.data(), nread);
}

void http_connection_base::do_write() {

    asio::async_write(client, asio::buffer(context->response->message),
                      [this](std::error_code ec, std::size_t /*length*/)
                      {
                          if (!ec)
                          {
                              client.close();
                          }
                      });

}
