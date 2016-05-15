//
// Created by kamlesh on 23/10/15.
//

#include <sstream>
#include <iostream>
#include <iterator>
#include <regex>
#include "settings.h"
#include "logger.h"
#include "resource_handler.h"

using namespace pigeon;
using namespace std;

resource_handler::resource_handler() {
    resource_location = settings::resource_location;
    default_page = settings::default_page;
}

resource_handler::~resource_handler() {

}

void resource_handler::get(http_context *context) {

    try {

        std::string request_path;

        if (!url_decode(context->request->url, request_path)) {
            context->request->create_response("Resource you requested cannot be found on the server!", context->response, HttpStatus::NotFound);
            return;
        }

        if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos) {
            context->request->create_response("Resource you requested cannot be found on the server!", context->response, HttpStatus::NotFound);
            return;
        }

        if (request_path[request_path.size() - 1] == '/') {
            request_path += default_page;
        }

        std::string full_path = resource_location + request_path;

        file_info fi(full_path);
        cache::get()->get_item(fi);

        if (fi.file_size == 0) {
            context->request->create_response("Resource you requested cannot be found on the server!", context->response, HttpStatus::NotFound);
            return;
        }

        ///check if the file is modified, if not send status 304
        string if_modified_since = context->request->get_header("If-Modified-Since");

        if (if_modified_since.size() > 0) {
            if (if_modified_since == fi.last_write_time) {
                context->request->create_response("", context->response, HttpStatus::NotModified);
                return;
            }
        }


        context->request->create_response(fi, context->response, HttpStatus::OK);


    }
    catch (std::exception &ex) {
        logger::get()->write(LogType::Error, Severity::Critical, ex.what());
        context->request->create_response(ex.what(), context->response, HttpStatus::InternalTcpServerError);
    }

}

void resource_handler::options(http_context *context) {
    context->request->create_response(context->response->message, context->response, HttpStatus::OK, false);
}

void resource_handler::process(http_context *context) {

    switch (context->request->method) {
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

