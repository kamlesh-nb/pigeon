//
// Created by kamlesh on 23/10/15.
//

#include <sstream>
#include <logger.h>
#include <http_context.h>
#include <http_handler.h>

using namespace pigeon;



http_handler::~http_handler() {

}

void http_handler::get(http_context *context) {

    try {

        settings * appSettings = context->application->get_settings();
        cache * resourceCache = context->application->get_resource_cache();
        resource_location = appSettings->get_resource_location();

        std::string request_path;
        if (!url_decode(context->request->url, request_path))
        {
            prepare(HttpStatus::NotFound, context);
            return;
        }

        if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos)
        {
            prepare(HttpStatus::NotFound, context);
            return;
        }

        if (request_path[request_path.size() - 1] == '/')
        {
            request_path += appSettings->get_default_page();
        }

        std::string full_path = resource_location + request_path;

        file_info fi(full_path);
        resourceCache->get_item(full_path, fi);

        if (fi.file_size == 0){
            prepare(HttpStatus::NotFound, context);
            return;
        }

        ///check if the file is modified, if not send status 304
        string key = "If-Modified-Since";
        key_value_pair kvp_if_modified_since;
        kvp_if_modified_since.key = key;
        context->request->get_header(kvp_if_modified_since);

        if (kvp_if_modified_since.value.size() > 0){
            if (kvp_if_modified_since.value == fi.last_write_time){
                prepare(HttpStatus::NotModified, context);
                return;
            }
        }

        if (fi.file_size == 0){
            prepare(HttpStatus::NotFound, context);
            return;
        }

        prepare(HttpStatus::OK, context);

        ///check if http compression is enabled
        string key2("Accept-Encoding");
        key_value_pair kvp_accept_enc;
        kvp_accept_enc.key = key2;
        context->request->get_header(kvp_accept_enc);

        std::size_t pos;

        if (kvp_accept_enc.value.size() > 0) {
            pos = kvp_accept_enc.value.find("gzip");
        }
        else
        {
            pos = string::npos;
        }

        if (pos != string::npos){
            context->response->message += fi.compresses_cached_headers;
            context->response->content += fi.compressed_content;
        }
        else {
            context->response->message += fi.cached_headers;
            context->response->content += fi.content;
        }

        finish(HttpStatus::OK, context);

    }
    catch (std::exception& ex){

        logger::get(context->application)->write(LogType::Error, Severity::Critical, ex.what());

    }

}

void http_handler::post(http_context *context) {

}

void http_handler::put(http_context *context) {

}

void http_handler::del(http_context *context)  {

}

void http_handler::options(http_context *context) {

}

void http_handler::process(http_context *context) {

    switch (context->request->method)
    {
        case http_method::HTTP_GET:
            get(context);
            break;

        case http_method::HTTP_POST:
            post(context);
            break;

        case http_method::HTTP_PUT:
            put(context);
            break;

        case http_method::HTTP_DELETE:
            del(context);
            break;

        case http_method::HTTP_OPTIONS:
            options(context);
            break;
    }
}
