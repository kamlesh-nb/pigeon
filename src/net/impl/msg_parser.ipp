#ifndef MSG_PROCESSOR_IPP
#define MSG_PROCESSOR_IPP


#include <regex>
#include <net/http_server.h>
#include "http_server.h"
#include "multi_part_parser.h"
#include "http_context.h"
#include "settings.h"
#include "cache.h"

using namespace pigeon;
using namespace pigeon::net;

int on_message_begin(http_parser* parser){

    return  0;
}
int on_message_complete(http_parser* parser){
    client_t *client = (client_t *)parser->data;
    client->requestHandler->process(client->context);
    return  0;
}
int on_url(http_parser* parser, const char* at, size_t len){

    client_t *client = (client_t *)parser->data;
    if (at && client->context->request) {
        char *data = (char *)malloc(sizeof(char) * len + 1);
        strncpy(data, at, len);
        data[len] = '\0';
        client->context->request->url += data;
        free(data);
    }

    return 0;
}
int on_header_name(http_parser* parser, const char* at, size_t len){

    client_t *client = (client_t *)parser->data;
    if (at && client->context->request) {
        string s;
        client->temp = (char *)malloc(sizeof(char) * len + 1);

        strncpy(client->temp, at, len);
        client->temp[len] = '\0';

    }

    return 0;
}
int on_header_value(http_parser* parser, const char* at, size_t len){

    client_t *client = (client_t *)parser->data;
    if (at && client->context->request) {
        string key, value;
        char *data = (char *)malloc(sizeof(char) * len + 1);
        strncpy(data, at, len);
        data[len] = '\0';
        value += data;
        key += client->temp;
        free(data);
        free(client->temp);

        client->context->request->set_header(key, value);
    }
    return 0;
}
int on_body(http_parser* parser, const char* at, size_t len){

    client_t *client = (client_t *)parser->data;
    if (at && client->context->request) {
        for (size_t i = 0; i < len; ++i) {
            client->context->request->content.push_back(at[i]);
        }

    }
    return 0;
}
int on_headers_complete(http_parser* parser){

    client_t *client = (client_t *)parser->data;
    client->context->request->method = parser->method;
    client->context->request->http_major_version = parser->http_major;
    client->context->request->http_minor_version = parser->http_minor;
    return 0;
}

class http_server::msg_parser {
private:
    http_parser_settings parser_settings;
    void init_parser_callbacks(){
        parser_settings.on_message_begin = on_message_begin;
        parser_settings.on_url = on_url;
        parser_settings.on_body = on_body;
        parser_settings.on_header_field = on_header_name;
        parser_settings.on_header_value = on_header_value;
        parser_settings.on_headers_complete = on_headers_complete;
        parser_settings.on_message_complete = on_message_complete;
    }
public:
    msg_parser(){
        init_parser_callbacks();

    }
    ~msg_parser(){}
    void init(http_parser& parser, http_parser_type type){
        http_parser_init(&parser, type);
    }
    ssize_t parse(client_t* client,  char* buf, ssize_t length){
        ssize_t parsed = http_parser_execute(&client->parser, &parser_settings, buf, length);
        return parsed;
    }
};


#endif //MSG_PROCESSOR_IPP