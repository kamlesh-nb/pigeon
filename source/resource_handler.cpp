//
// Created by kamlesh on 23/10/15.
//

#include <sstream>
#include <logger.h>
#include "resource_handler.h"
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <regex>
#include <multi_part_parser.h>
#include <fstream>

using namespace pigeon;
using namespace std;


resource_handler::resource_handler() {
    resource_location = settings::resource_location;
    default_page = settings::default_page;
}

resource_handler::~resource_handler() {

}

void resource_handler::get(http_context* context) {

    try {

        std::string request_path;
        
		if (!url_decode(context->request->url, request_path))
        {
            context->request->create_response("Not Found!", *context->response, HttpStatus::NotFound); return;
        }

        if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos)
        {
            context->request->create_response("Not Found!", *context->response, HttpStatus::NotFound); return;
        }

        if (request_path[request_path.size() - 1] == '/')
        {
            request_path += default_page;
        }

        std::string full_path = resource_location + request_path;

        file_info fi(full_path);
        cache::get()->get_item(full_path, fi);

        if (fi.file_size == 0){
            context->request->create_response("Not Found!", *context->response, HttpStatus::NotFound); return;
        }

        ///check if the file is modified, if not send status 304
        string key = "If-Modified-Since";
        key_value_pair kvp_if_modified_since;
        kvp_if_modified_since.key = key;
        context->request->get_header(kvp_if_modified_since);

        if (kvp_if_modified_since.value.size() > 0){
            if (kvp_if_modified_since.value == fi.last_write_time){
                context->request->create_response("Not Modified!", *context->response, HttpStatus::NotModified); return;
            }
        }

        ///check if http compression is accepted
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
            context->request->create_response(fi.compresses_cached_headers, fi.compressed_content, *context->response, HttpStatus::OK);
        }
        else {
            context->request->create_response(fi.cached_headers, fi.content, *context->response, HttpStatus::OK);
        }

    }
    catch (std::exception& ex){
        logger::get()->write(LogType::Error, Severity::Critical, ex.what());
        context->request->create_response(ex.what(), *context->response, HttpStatus::InternalTcpServerError);
    }

}

void resource_handler::post(http_context* context) {

    //"multipart/form-data; boundary=----WebKitFormBoundaryOJ0iKrrnEKPOxjBy"

    string key1("Content-Type");
    key_value_pair kvp_content_type;
    kvp_content_type.key = key1;
    context->request->get_header(kvp_content_type);


    if(kvp_content_type.value.size() > 0){
        replace(kvp_content_type.value.begin(), kvp_content_type.value.end(), ';', ' ');
        std::istringstream issParams(kvp_content_type.value.c_str());
        vector<string> vparams{istream_iterator<string>{issParams},
                               istream_iterator<string>{}};

        if(vparams[0] == "multipart/form-data") {

            //std::cout << context->request->content.size() << std::endl;

            multi_part_parser mpp;
            mpp.execute_parser(context);

            ofstream uploaded_file;
            string file_path = settings::file_upload_location;
            file_path += "/";
            file_path += context->request->form_data.parameters["filename"];
            uploaded_file.open(file_path.c_str(), ios::app);
            uploaded_file << context->request->form_data.filedata << endl;
            uploaded_file.flush();
            uploaded_file.close();

        }
    }





    //"multipart/form-data; boundary=----WebKitFormBoundaryOJ0iKrrnEKPOxjBy"

    context->request->create_response("Not Implemented!", *context->response, HttpStatus::NotImplemented);
}

void resource_handler::put(http_context* context) {
    context->request->create_response("Not Implemented!", *context->response, HttpStatus::NotImplemented);
}

void resource_handler::del(http_context* context)  {
    context->request->create_response("Not Implemented!", *context->response, HttpStatus::NotImplemented);
}

void resource_handler::options(http_context* context) {

    context->request->create_response("Not Implemented!", *context->response, HttpStatus::NotImplemented);

}

void resource_handler::process(http_context* context) {

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

