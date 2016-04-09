#include <request_processor.h>
#include <settings.h>
#include <http_filters.h>
#include <multi_part_parser.h>
#include <http_handlers.h>
#include <regex>
#include <iostream>

using namespace std;
using namespace pigeon;

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

request_processor::request_processor()
{
    allowed_origins_cfg = settings::cors_headers["Access-Control-Allow-Origin"];
    allowed_methods_cfg = settings::cors_headers["Access-Control-Allow-Methods"];
    allowed_headers_cfg = settings::cors_headers["Access-Control-Allow-Headers"];
    allowed_max_age_cfg = settings::cors_headers["Access-Control-Max-Age"];
    allowed_credential_cfg = settings::cors_headers["Access-Control-Allow-Credentials"];
    allowed_expose_headers_cfg = settings::cors_headers["Access-Control-Expose-Headers"];



    allowed_origins_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Origin);
    allowed_methods_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Methods);
    allowed_headers_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Headers);
    allowed_max_age_hdr_fld = get_header_field(HttpHeader::Access_Control_Max_Age);
    allowed_credentials_hdr_fld = get_header_field(HttpHeader::Access_Control_Allow_Credentials);
    allowed_expose_headers_hdr_fld = get_header_field(HttpHeader::Access_Control_Expose_Headers);


}

request_processor::~request_processor()
{
}

void request_processor::process(http_context *context)
{

    //check if request is for web api
    context->request->is_api = is_api(context->request->url, settings::api_route);

    //parse cookies if any
    parse_cookies(context);

    //parse url for any query string parameters
    parse_url(context);

    //check if content type is multipart/form-data, parse content if it is
    parse_multipart(context);

    //execute if there are any request filters registered,
    //if any of the filter fails, it will terminate the further processing of request
    if(!execute_request_filters(context)){
        return;
    }

    //if cors is enabled for the service, deal with it
    if(settings::enable_cors){
        handle_cors(context);
    }

    if ((context->request->is_api) &
            (context->request->method == HTTP_GET ||
            context->request->method == HTTP_PUT ||
            context->request->method == HTTP_POST ||
            context->request->method == HTTP_DELETE)) {
        //process the request if it is for web api
        auto handler = http_handlers::instance()->get(context->request->url);
        handler->process(context);
    }
    else
    {
        //process the request if it is for static content
        auto handler = http_handlers::instance()->get("resource");
        handler->process(context);
    }



    //execute if there are any response filters registered,
    //if any of the filter fails, it will terminate the further processing of request
    if(!execute_response_filters(context)){
        return;
    }

}

void request_processor::parse_multipart(http_context *context)
{

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
            multi_part_parser mpp;
            mpp.parse(context, boundary);
        }

    }
}

bool request_processor::execute_request_filters(http_context *context)
{
    bool retval = true;

    for(auto& filter:settings::request_filters){
        auto flt = http_filters::instance()->get(filter);
        if(flt) {
            if(!flt->execute(context)) {
                retval = false;
                break;
            }
        }
    }
    return retval;

}

bool request_processor::execute_response_filters(http_context *context)
{
    bool retval = true;

    for(auto& filter:settings::response_filters){
        auto flt = http_filters::instance()->get(filter);
        if(flt) {
            if(!flt->execute(context)) {
                retval = false;
                break;
            }
        }
    }
    return retval;

}

void request_processor::handle_cors(http_context *context)
{
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
                context->response->set_header(allowed_credentials_hdr_fld, allowed_credential_cfg);

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

void request_processor::parse_cookies(http_context *context)
{

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

void request_processor::parse_url(http_context *context)
{

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
                        value.push_back(c);
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

bool request_processor::is_api(string &Uri, string &apiroute)
{
    std::size_t pos = Uri.find(apiroute);
    return pos != string::npos;
}