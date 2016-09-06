#ifndef MSG_PARSER
#define MSG_PARSER

#include <http_server.h>
#include "http_server.h"

using namespace pigeon;

int on_message_begin(http_parser* parser){

    return  0;
}

int on_message_complete(http_parser* parser){
    client_t *client = (client_t *)parser->data;
    client->context->data = client;

    client->reqproc_ptr->process(client->context, [](http_context* context){

            client_t* client = (client_t*)context->data;
            uv_buf_t resbuf;
            resbuf.base = (char*)client->context->response->buffer->to_cstr();
            resbuf.len = (unsigned long)client->context->response->buffer->get_length();

            client->write_req.data = client;
            if (uv_is_writable((uv_stream_t*)&client->stream)) {
                int r = uv_write(&client->write_req,
                                 (uv_stream_t *) &client->stream,
                                 &resbuf,
                                 1,
                                 [](uv_write_t *req, int status) {
                                     if (!uv_is_closing((uv_handle_t *) req->handle)) {
                                         uv_close((uv_handle_t *) req->handle,
                                                  [](uv_handle_t *handle) {
                                                      client_t *client = (client_t *) handle->data;
                                                      client->context->response->buffer->clear();
                                                      delete client->context->response->buffer;
                                                      delete client->context;
                                                      free(client);
                                                  });
                                     }


                                 });
            } else {
                uv_close((uv_handle_t *)&client->stream,
                         [](uv_handle_t *handle) {
                             client_t *client = (client_t *) handle->data;
                             client->context->response->buffer->clear();
                             delete client->context->response->buffer;
                             delete client->context;
                             free(client);
                         });
            }

        return 0;
    });
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
public:
    msg_parser(){ init_parser_callbacks(); }
    ~msg_parser(){}
    void init_parser_callbacks(){
        parser_settings.on_message_begin = on_message_begin;
        parser_settings.on_url = on_url;
        parser_settings.on_body = on_body;
        parser_settings.on_header_field = on_header_name;
        parser_settings.on_header_value = on_header_value;
        parser_settings.on_headers_complete = on_headers_complete;
        parser_settings.on_message_complete = on_message_complete;
    }
    void init(http_parser& parser, http_parser_type type){
        http_parser_init(&parser, type);
    }
    ssize_t parse(client_t* client,  char* buf, ssize_t length){
        ssize_t parsed = http_parser_execute(&client->parser, &parser_settings, buf, length);
        return parsed;
    }
};


#endif //MSG_PARSER