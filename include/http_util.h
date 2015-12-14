#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#include <string>
#include <uv.h>
#include "http_msg.h"

using namespace std;

namespace pigeon {




	string get_cached_response(bool is_api);
	char *now();
	string get_header_field(HttpHeader hdr);
	string get_status_phrase(HttpStatus status);
	string get_status_msg(HttpStatus status);
	string get_err_msg(HttpStatus status);
	string get_mime_type(string &extension);
	string get_log_type(LogType type);
	string get_severity(Severity severe);
    bool is_api(string &Uri);
    void parse_query_string(http_request &req);
    bool url_decode(const string &in, string &out);



}



#endif //HTTP_UTIL_H

