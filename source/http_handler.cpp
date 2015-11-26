//
// Created by kamlesh on 23/10/15.
//

#include <sstream>
#include <logger.h>
#include <http_handler.h>
#include <iostream>

using namespace pigeon;
using namespace pigeon::tcp;
using namespace std;


http_handler::http_handler() {

    
    resource_location = settings::resource_location;
    default_page = settings::default_page;

}

http_handler::~http_handler() {

}

void http_handler::get(http_request* request) {

    try {


		std::string message;
        std::string request_path;
        
		if (!url_decode(request->url, request_path))
        {
			request->create_response("Not Found!", HttpStatus::NotFound); return;
        }

        if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos)
        {
			request->create_response("Not Found!", HttpStatus::NotFound); return;
        }

        if (request_path[request_path.size() - 1] == '/')
        {
            request_path += default_page;
        }

        std::string full_path = resource_location + request_path;

        file_info fi(full_path);
        cache::get()->get_item(full_path, fi);

        if (fi.file_size == 0){
			request->create_response("Not Found!", HttpStatus::NotFound); return;
        }

        ///check if the file is modified, if not send status 304
        string key = "If-Modified-Since";
        key_value_pair kvp_if_modified_since;
        kvp_if_modified_since.key = key;
        request->get_header(kvp_if_modified_since);

        if (kvp_if_modified_since.value.size() > 0){
            if (kvp_if_modified_since.value == fi.last_write_time){
				request->create_response("Not Modified!", HttpStatus::NotModified); return;
            }
        }

        ///check if http compression is accepted
        string key2("Accept-Encoding");
        key_value_pair kvp_accept_enc;
        kvp_accept_enc.key = key2;
        request->get_header(kvp_accept_enc);

        std::size_t pos;

        if (kvp_accept_enc.value.size() > 0) {
            pos = kvp_accept_enc.value.find("gzip");
        }
        else
        {
            pos = string::npos;
        }

        if (pos != string::npos){
			request->create_response(fi.compresses_cached_headers, fi.compressed_content, HttpStatus::OK);
        }
        else {
			request->create_response(fi.cached_headers, fi.content, HttpStatus::OK);
        }

    }
    catch (std::exception& ex){
        logger::get()->write(LogType::Error, Severity::Critical, ex.what());
		request->create_response(ex.what(), HttpStatus::OK);
    }

}

void http_handler::post(http_request* request) {
	request->create_response("Not Implemented!", HttpStatus::NotImplemented);
}

void http_handler::put(http_request* request) {
	request->create_response("Not Implemented!", HttpStatus::NotImplemented);
}

void http_handler::del(http_request* request)  {
	request->create_response("Not Implemented!", HttpStatus::NotImplemented);
}

void http_handler::options(http_request* request) {

	request->create_response("Not Implemented!", HttpStatus::NotImplemented);

}

void http_handler::process(http_request* request) {

    switch (request->method)
    {
        case http_method::HTTP_GET:
			get(request);
            break;

        case http_method::HTTP_POST:
			post(request);
            break;

        case http_method::HTTP_PUT:
			put(request);
            break;

        case http_method::HTTP_DELETE:
			del(request);
            break;

        case http_method::HTTP_OPTIONS:
			options(request);
            break;
    }
}
