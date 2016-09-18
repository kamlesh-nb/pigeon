//
// Created by kamlesh on 9/17/16.
//

#include <regex>
#include <http_parser.h>
#include <net/http_server.h>
#include "settings.h"
#include "cache.h"
#include "request_handler.h"
#include "http_server.h"

using namespace pigeon;
using namespace pigeon::net;


enum url_state {
    url_start,
    param_name_start,
    param_value_start
} url_state_;

enum cookie_state {
    cookie_start,
    key_start,
    value_start
} cookie_state_;

request_handler::request_handler(cache* p_cache) : m_cache (p_cache){
    auto cors_hearders = settings::cors_headers;
    //one time loading of all the cors headers cosfigured in service.json
    allowed_origins_cfg = cors_hearders["Access-Control-Allow-Origin"];
    allowed_methods_cfg = cors_hearders["Access-Control-Allow-Methods"];
    allowed_headers_cfg = cors_hearders["Access-Control-Allow-Headers"];
    allowed_max_age_cfg = cors_hearders["Access-Control-Max-Age"];
    allowed_credential_cfg = cors_hearders["Access-Control-Allow-Credentials"];
    allowed_expose_headers_cfg = cors_hearders["Access-Control-Expose-Headers"];

    //one time loading of all the header names that has to be sent for cors requests
    allowed_origins_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Origin);
    allowed_methods_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Methods);
    allowed_headers_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Headers);
    allowed_max_age_hdr_fld = get_header_field(HttpHeader::Access_Control_Max_Age);
    allowed_credentials_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Credentials);
    allowed_expose_headers_hdr_fld = get_header_field(HttpHeader::Access_Control_Expose_Headers);

    resource_location = settings::resource_location;
    default_page = settings::default_page;
}
request_handler::~request_handler() {

}
int request_handler::process(http_context *context){

    //check if request is for web api
    context->request->is_api = is_api(context->request->url, settings::api_route);

    //parse cookies if any
    parse_cookies(context);

    //parse url for R string parameters
    parse_url(context);

    //check if content type is multipart/form-data, parse content if it is
    parse_multipart(context);

    //execute if there are any request filters registered,
    //if any of the filter fails, it will terminate the further
    //processing of request
    if(!execute_request_filters(context)){
        return -1;
    }

    //if cors is enabled for the service, deal with it
    if(settings::enable_cors){
        handle_cors(context);
    }

    handle_request(context);

    //execute if there are any response filters registered,
    //if any of the filter fails, it will terminate the further
    //processing of request
//    if(!execute_response_filters(context)){
//        return -1;
//    }
//
//    func(context);

}
int request_handler::handle_request(http_context *context){

    if ((context->request->is_api) &
        (context->request->method == HTTP_GET ||
         context->request->method == HTTP_PUT ||
         context->request->method == HTTP_POST ||
         context->request->method == HTTP_DELETE)) {

        switch (context->request->method){
            case HTTP_GET:
                get_func[context->request->url](context);
                break;
            case HTTP_PUT:
                put_func[context->request->url](context);
                break;
            case HTTP_POST:
                post_func[context->request->url](context);
                break;
            case HTTP_DELETE:
                del_func[context->request->url](context);
                break;
            default:
                break;
        }
    }
    else
    {
        get_resource(context);
    }

}
int request_handler::parse_multipart(http_context *context){

    //check if the request if really for multipart/form-data
    string content_type = context->request->get_header("Content-Type");

    if (content_type.size() > 0) {

        std::regex re(";|=|\\+s");
        std::sregex_token_iterator p(content_type.begin(), content_type.end(), re, -1);
        std::sregex_token_iterator end;
        string val, boundary;
        val += *p++;

        if (val == "multipart/form-data") {
            *p++;
            boundary += *p++;

            mpp.parse(context, boundary);
        }

    }
}
int request_handler::execute_request_filters(http_context *context){
    bool retval = true;
    vector<string> request_filters = settings::request_filters;
    for(auto& filter:request_filters){
        auto flt = filter_func[filter];
        if(flt) {
            if(!flt(context)) {
                retval = false;
                break;
            }
        }
    }
    return retval;

}
int request_handler::execute_response_filters(http_context *context){
    bool retval = true;
    vector<string> response_filters = settings::response_filters;
    for(auto& filter:response_filters){
        auto flt = filter_func[filter];
        if(flt) {
            if(!flt(context)) {
                retval = false;
                break;
            }
        }
    }
    return retval;

}
int request_handler::handle_cors(http_context *context){
    string origin = context->request->get_header("Origin");
    string host = context->request->get_header("Host");

    unsigned long pos = origin.find(host);


    if(pos == string::npos){
        unsigned long allowed_origin_pos = allowed_origins_cfg.find(origin);

        if((allowed_origin_pos != string::npos) || (allowed_origins_cfg == "*") ){

            if(context->request->method == HTTP_GET ||
               context->request->method == HTTP_HEAD ||
               context->request->method == HTTP_POST ||
               context->request->method == HTTP_PUT){

                context->response->set_header(allowed_origins_hdr_fld, origin);

                if (context->request->has_cookies()) {
                    context->response->set_header(allowed_credentials_hdr_fld, allowed_credential_cfg);
                }

            } else if (context->request->method == HTTP_OPTIONS) {

                context->response->set_header(allowed_origins_hdr_fld, origin);
                context->response->set_header(allowed_methods_hdr_fld, allowed_methods_cfg);
                context->response->set_header(allowed_headers_hdr_fld, allowed_headers_cfg);
                context->response->set_header(allowed_max_age_hdr_fld, allowed_max_age_cfg);
                context->response->set_header(allowed_credentials_hdr_fld, allowed_credential_cfg);
            }

        }

    }

}
int request_handler::parse_cookies(http_context *context){

    cookie_state_ = key_start;
    string cookies = context->request->get_header("Cookie");

    string key, value;

    for(auto& c:cookies){

        switch (cookie_state_){

            case cookie_start:
                cookie_state_ = key_start;
                break;
            case key_start:
                if(c == '='){
                    cookie_state_ = value_start;
                } else {
                    if (c != ' ') {
                        key.push_back(c);
                    }
                }
                break;

            case value_start:
                if(c == ';'){
                    cookie_state_ = key_start;
                    context->request->set_cookie(key, value);
                    key.clear(); value.clear();
                }
                else {
                    value.push_back(c);
                }
                break;
        }
    }
    if(key.size() > 0 && value.size() > 0){
        context->request->set_cookie(key, value);
        key.clear(); value.clear();
    }

}
int request_handler::parse_url(http_context *context){

    url_state_ = url_start;
    string url_source = context->request->url;
    string key, value, uri;

    for(auto& c:url_source){

        switch(url_state_){

            case url_start:
                if (c == '?')
                {
                    url_state_ = param_name_start;
                } else {
                    uri.push_back(c);
                }
                break;
            case param_name_start:
                if(c == '='){
                    url_state_ = param_value_start;
                } else {
                    key.push_back(c);
                }
                break;

            case param_value_start:
                if(c == '&'){
                    url_state_ = param_name_start;
                    context->request->set_parameter(key, value);
                    key.clear(); value.clear();
                }
                else {
                    if(c != '='){
                        value.push_back(c);
                    }

                }
                break;

        }

    }
    context->request->url = uri;
    if(key.size() > 0 && value.size() > 0){
        context->request->set_parameter(key, value);
        key.clear(); value.clear();
    }

}
bool request_handler::is_api(string &Uri, string &apiroute){
    std::size_t pos = Uri.find(apiroute);
    return pos != string::npos;
}
int request_handler::finish_process(http_context* context){

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
}
int request_handler::get_resource(http_context *context) {

    std::string request_path;

    if (!url_decode(context->request->url, request_path)) {
        context->request->create_response("Resource you requested cannot be found on the server!", context->response, HttpStatus::NotFound);
        return -1;
    }

    if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos) {
        context->request->create_response("Resource you requested cannot be found on the server!", context->response, HttpStatus::NotFound);
        return -1;
    }

    if (request_path[request_path.size() - 1] == '/') {
        request_path += default_page;
    }

    std::string full_path = resource_location + request_path;

    string if_modified_since = context->request->get_header("If-Modified-Since");

    file_info* fi = m_cache->get_item(full_path);

    if(!fi){
        context->request->create_response("Resource you requested cannot be found on the server!", context->response, HttpStatus::NotFound);
        return -1;
    }

    if (fi->file_size == 0) {
        context->request->create_response("Resource you requested cannot be found on the server!", context->response, HttpStatus::NotFound);
        return -1;
    }

    ///check if the file is modified, if not send status 304


    if (if_modified_since.size() > 0) {
        if (if_modified_since == fi->last_write_time) {
            context->request->create_response("", context->response, HttpStatus::NotModified);
            return -1;
        }
    }


    context->request->create_response(*fi, context->response, HttpStatus::OK);

    return 0;

}
void request_handler::add_route(std::string route, int method, request_process_step_cb func){
    switch (method){
        case 0:
            del_func.insert({route, func});
            break;
        case 1:
            get_func.insert({route, func});
            break;
        case 3:
            post_func.insert({route, func});
            break;
        case 4:
            put_func.insert({route, func});
            break;
        default:
            break;
    }

}
void request_handler::add_filter(std::string filter, request_process_step_cb func){
    filter_func.insert({filter, func});
}