//
// Created by kamlesh on 9/17/16.
//

#include <regex>
#include <http_parser.h>
#include <net/HttpServer.h>
#include "Settings.h"
#include "FileCache.h"
#include "RequestHandler.h"
#include "HttpServer.h"

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

RequestHandler::RequestHandler(FileCache* p_cache) : m_cache (p_cache){
    auto cors_hearders = Settings::CORSHeaders;
    //one time loading of all the cors headers cosfigured in service.json
    allowed_origins_cfg = cors_hearders["Access-Control-Allow-Origin"];
    allowed_methods_cfg = cors_hearders["Access-Control-Allow-Methods"];
    allowed_headers_cfg = cors_hearders["Access-Control-Allow-Headers"];
    allowed_max_age_cfg = cors_hearders["Access-Control-Max-Age"];
    allowed_credential_cfg = cors_hearders["Access-Control-Allow-Credentials"];
    allowed_expose_headers_cfg = cors_hearders["Access-Control-Expose-Headers"];

    //one time loading of all the header names that has to be sent for cors requests
    allowed_origins_hdr_fld = GetHeaderField(HttpHeader::Access_Control_Allow_Origin);
    allowed_methods_hdr_fld = GetHeaderField(HttpHeader::Access_Control_Allow_Methods);
    allowed_headers_hdr_fld = GetHeaderField(HttpHeader::Access_Control_Allow_Headers);
    allowed_max_age_hdr_fld = GetHeaderField(HttpHeader::Access_Control_Max_Age);
    allowed_credentials_hdr_fld = GetHeaderField(HttpHeader::Access_Control_Allow_Credentials);
    allowed_expose_headers_hdr_fld = GetHeaderField(HttpHeader::Access_Control_Expose_Headers);

    resource_location = Settings::ResourceLocation;
    default_page = Settings::DefaultPage;
}
RequestHandler::~RequestHandler() {

}
int RequestHandler::StartProcess(HttpContext *context){

    //check if request is for web api
    context->Request->is_api = IsApi(context->Request->url, Settings::ApiRoute);

    //parse cookies if any
    ParseCookies(context);

    //parse url for QueryContext string parameters
    ParseUrl(context);

    //check if content type is multipart/form-data, parse content if it is
    MarseMultiPartData(context);

    //execute if there are any request filters registered,
    //if any of the filter fails, it will terminate the further
    //processing of request
    if(!ExecuteRequestFilters(context)){
        return -1;
    }

    //if cors is enabled for the service, deal with it
    if(Settings::EnableCORS){
        HandleCORSHeaders(context);
    }

    HandleRequest(context);

    //execute if there are any response filters registered,
    //if any of the filter fails, it will terminate the further
    //processing of request
//    if(!execute_response_filters(context)){
//        return -1;
//    }
//
//    func(context);

}
int RequestHandler::HandleRequest(HttpContext *context){

    if ((context->Request->is_api) &
        (context->Request->method == HTTP_GET ||
         context->Request->method == HTTP_PUT ||
         context->Request->method == HTTP_POST ||
         context->Request->method == HTTP_DELETE)) {

        switch (context->Request->method){
            case HTTP_GET:
                get_func[context->Request->url](context);
                break;
            case HTTP_PUT:
                put_func[context->Request->url](context);
                break;
            case HTTP_POST:
                post_func[context->Request->url](context);
                break;
            case HTTP_DELETE:
                del_func[context->Request->url](context);
                break;
            default:
                break;
        }
    }
    else
    {
        GetResource(context);
    }

}
int RequestHandler::MarseMultiPartData(HttpContext *context){

    //check if the request if really for multipart/form-data
    string content_type = context->Request->GetHeader("Content-Type");

    if (content_type.size() > 0) {

        std::regex re(";|=|\\+s");
        std::sregex_token_iterator p(content_type.begin(), content_type.end(), re, -1);
        std::sregex_token_iterator end;
        string val, boundary;
        val += *p++;

        if (val == "multipart/form-data") {
            *p++;
            boundary += *p++;

            mpp.Parse(context, boundary);
        }

    }
}
int RequestHandler::ExecuteRequestFilters(HttpContext *context){
    bool retval = true;
    vector<string> request_filters = Settings::RequestFilters;
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
int RequestHandler::ExecuteResponseFilters(HttpContext *context){
    bool retval = true;
    vector<string> response_filters = Settings::ResponseFilters;
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
int RequestHandler::HandleCORSHeaders(HttpContext *context){
    string origin = context->Request->GetHeader("Origin");
    string host = context->Request->GetHeader("Host");

    unsigned long pos = origin.find(host);


    if(pos == string::npos){
        unsigned long allowed_origin_pos = allowed_origins_cfg.find(origin);

        if((allowed_origin_pos != string::npos) || (allowed_origins_cfg == "*") ){

            if(context->Request->method == HTTP_GET ||
               context->Request->method == HTTP_HEAD ||
               context->Request->method == HTTP_POST ||
               context->Request->method == HTTP_PUT){

                context->Response->SetHeader(allowed_origins_hdr_fld, origin);

                if (context->Request->HasCookies()) {
                    context->Response->SetHeader(allowed_credentials_hdr_fld, allowed_credential_cfg);
                }

            } else if (context->Request->method == HTTP_OPTIONS) {

                context->Response->SetHeader(allowed_origins_hdr_fld, origin);
                context->Response->SetHeader(allowed_methods_hdr_fld, allowed_methods_cfg);
                context->Response->SetHeader(allowed_headers_hdr_fld, allowed_headers_cfg);
                context->Response->SetHeader(allowed_max_age_hdr_fld, allowed_max_age_cfg);
                context->Response->SetHeader(allowed_credentials_hdr_fld, allowed_credential_cfg);
            }

        }

    }

}
int RequestHandler::ParseCookies(HttpContext *context){

    cookie_state_ = key_start;
    string cookies = context->Request->GetHeader("Cookie");

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
                    context->Request->SetCookie(key, value);
                    key.clear(); value.clear();
                }
                else {
                    value.push_back(c);
                }
                break;
        }
    }
    if(key.size() > 0 && value.size() > 0){
        context->Request->SetCookie(key, value);
        key.clear(); value.clear();
    }

}
int RequestHandler::ParseUrl(HttpContext *context){

    url_state_ = url_start;
    string url_source = context->Request->url;
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
                    context->Request->SetParameter(key, value);
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
    context->Request->url = uri;
    if(key.size() > 0 && value.size() > 0){
        context->Request->SetParameter(key, value);
        key.clear(); value.clear();
    }

}
bool RequestHandler::IsApi(string &Uri, string &apiroute){
    std::size_t pos = Uri.find(apiroute);
    return pos != string::npos;
}
int RequestHandler::FinishProcess(HttpContext *context){

    client_t* client = (client_t*)context->data;
    uv_buf_t resbuf;
    resbuf.base = (char*) client->context->Response->buffer->ToCStr();
    resbuf.len = (unsigned long) client->context->Response->buffer->GetLength();

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
                                              client->context->Response->buffer->Clear();
                                              delete client->context->Response->buffer;
                                              delete client->context;
                                              free(client);
                                          });
                             }


                         });
    } else {
        uv_close((uv_handle_t *)&client->stream,
                 [](uv_handle_t *handle) {
                     client_t *client = (client_t *) handle->data;
                     client->context->Response->buffer->Clear();
                     delete client->context->Response->buffer;
                     delete client->context;
                     free(client);
                 });
    }

    return 0;
}
int RequestHandler::GetResource(HttpContext *context) {

    std::string request_path;

    if (!UrlDecode(context->Request->url, request_path)) {
        context->Request->CreateResponse("Resource you requested cannot be found on the server!", context->Response,
                                         HttpStatus::NotFound);
        return -1;
    }

    if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos) {
        context->Request->CreateResponse("Resource you requested cannot be found on the server!", context->Response,
                                         HttpStatus::NotFound);
        return -1;
    }

    if (request_path[request_path.size() - 1] == '/') {
        request_path += default_page;
    }

    std::string full_path = resource_location + request_path;

    string if_modified_since = context->Request->GetHeader("If-Modified-Since");

    FileInfo* fi = m_cache->GetItem(full_path);

    if(!fi){
        context->Request->CreateResponse("Resource you requested cannot be found on the server!", context->Response,
                                         HttpStatus::NotFound);
        return -1;
    }

    if (fi->file_size == 0) {
        context->Request->CreateResponse("Resource you requested cannot be found on the server!", context->Response,
                                         HttpStatus::NotFound);
        return -1;
    }

    ///check if the file is modified, if not send status 304


    if (if_modified_since.size() > 0) {
        if (if_modified_since == fi->last_write_time) {
            context->Request->CreateResponse("", context->Response, HttpStatus::NotModified);
            return -1;
        }
    }


    context->Request->CreateResponse(*fi, context->Response, HttpStatus::OK);

    return 0;

}
void RequestHandler::AddRoute(std::string route, int method, OnHandlerExecution func){
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
void RequestHandler::AddFilter(std::string filter, OnHandlerExecution func){
    filter_func.insert({filter, func});
}