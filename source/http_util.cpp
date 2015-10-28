#include "http_util.h"
#include <ctime>
#include <string.h>
#include <chrono>
#include <sstream>
#include <http_handler.h>
#include <rest_handlers.h>
#include <iterator>

using namespace pigeon;

    struct logtype {
        int log_id;
        const char *log_type;
    } logtypes[] =
            {
                    {1, "[WARNING]"},
                    {2, "[INFORMATION]"},
                    {3, "[ERROR]"},
                    {0, 0}
            };

    struct severity {
        int severity_id;
        const char *severity_type;
    } severities[] =
            {
                    {1, "[LOW]"},
                    {2, "[MEDIUM]"},
                    {3, "[HIGH]"},
                    {4, "[CRITICAL]"},
                    {0, 0}
            };

    struct header {
        int header_id;
        const char *header_field;
    } headers[] =
            {
                    {1, "Content-Type: "},
                    {2, "Content-Length: "},
                    {3, "Last-Modified: "},
                    {4, "ETag: "},
                    {5, "Content-Encoding: deflate\r\n"},
                    {0, 0}
            };

    struct statusphrase {
        int status_code;
        const char *status_phrase;
    } statusphrases[] =
            {
                    {100, "HTTP/1.1 100 Continue"},
                    {101, "HTTP/1.1 101 Switching Protocols"},
                    {103, "HTTP/1.1 103 Checkpoint"},
                    {200, "HTTP/1.1 200 OK"},
                    {201, "HTTP/1.1 201 Created"},
                    {202, "HTTP/1.1 202 Accepted"},
                    {203, "HTTP/1.1 203 Non-Authoritative-Information"},
                    {204, "HTTP/1.1 204 No Content"},
                    {205, "HTTP/1.1 205 Reset Content"},
                    {206, "HTTP/1.1 206 Partial Content"},
                    {300, "HTTP/1.1 300 Multiple Choices"},
                    {301, "HTTP/1.1 301 Moved Permanently"},
                    {302, "HTTP/1.1 302 Found"},
                    {303, "HTTP/1.1 303 See Other"},
                    {304, "HTTP/1.1 304 Not Modified"},
                    {306, "HTTP/1.1 306 Switch Proxy"},
                    {307, "HTTP/1.1 307 Temporary Redirect"},
                    {308, "HTTP/1.1 308 Resume Incomplete"},
                    {400, "HTTP/1.1 400 Bad Request"},
                    {401, "HTTP/1.1 401 Unauthorized"},
                    {402, "HTTP/1.1 402 Payment Required"},
                    {403, "HTTP/1.1 403 Forbidden"},
                    {404, "HTTP/1.1 404 Not Found"},
                    {405, "HTTP/1.1 405 Method Not Allowed"},
                    {406, "HTTP/1.1 406 Not Acceptable"},
                    {407, "HTTP/1.1 407 Proxy Authentication Required"},
                    {408, "HTTP/1.1 408 Request Timeout"},
                    {409, "HTTP/1.1 409 Conflict"},
                    {410, "HTTP/1.1 410 Gone"},
                    {411, "HTTP/1.1 411 Length Required"},
                    {412, "HTTP/1.1 412 Precondition Failed"},
                    {413, "HTTP/1.1 413 Request Entity Too Large"},
                    {414, "HTTP/1.1 414 Request URI Too Large"},
                    {415, "HTTP/1.1 415 Unsupported Media Type"},
                    {416, "HTTP/1.1 416 Requested Range Not Satisfiable"},
                    {417, "HTTP/1.1 417 Expectation Failled"},
                    {500, "HTTP/1.1 500 Internal Server Error"},
                    {501, "HTTP/1.1 501 Not Implemented"},
                    {502, "HTTP/1.1 502 Bad Gateway"},
                    {503, "HTTP/1.1 503 Service Unavailable"},
                    {504, "HTTP/1.1 504 Gateway Timeout"},
                    {505, "HTTP/1.1 505 HTTP Version Not Supported"},
                    {511, "HTTP/1.1 511 Network Authentication Required"},
                    {0,   0}
            };

    struct statusmsg {
        int status_code;
        const char *status_msg;
    } statusmsgs[] =
            {
                    {100, "Continue"},
                    {101, "Switching Protocols"},
                    {103, "Checkpoint"},
                    {200, "OK"},
                    {201, "Created"},
                    {202, "Accepted"},
                    {203, "Non-Authoritative-Information"},
                    {204, "No Content"},
                    {205, "Reset Content"},
                    {206, "Partial Content"},
                    {300, "Multiple Choices"},
                    {301, "Moved Permanently"},
                    {302, "Found"},
                    {303, "See Other"},
                    {304, "Not Modified"},
                    {306, "Switch Proxy"},
                    {307, "Temporary Redirect"},
                    {308, "Resume Incomplete"},
                    {400, "Bad Request"},
                    {401, "Unauthorized"},
                    {402, "Payment Required"},
                    {403, "Forbidden"},
                    {404, "Not Found"},
                    {405, "Method Not Allowed"},
                    {406, "Not Acceptable"},
                    {407, "Proxy Authentication Required"},
                    {408, "Request Timeout"},
                    {409, "Conflict"},
                    {410, "Gone"},
                    {411, "Length Required"},
                    {412, "Precondition Failed"},
                    {413, "Request Entity Too Large"},
                    {414, "Request URI Too Large"},
                    {415, "Unsupported Media Type"},
                    {416, "Requested Range Not Satisfiable"},
                    {417, "Expectation Failled"},
                    {500, "Internal Server Error"},
                    {501, "Not Implemented"},
                    {502, "Bad Gateway"},
                    {503, "Service Unavailable"},
                    {504, "Gateway Timeout"},
                    {505, "HTTP Version Not Supported"},
                    {511, "Network Authentication Required"},
                    {0,   0}
            };

    struct mapping {
        const char *extension;
        const char *mime_type;
    } mappings[] =
            {
                    {"html",  "text / html; charset=UTF-8"},
                    {"htm",   "text/html; charset=UTF-8"},
                    {"htmls", "text/html; charset=UTF-8"},
                    {"jpe",   "image/jpeg"},
                    {"jpeg",  "image/jpeg"},
                    {"jpg",   "image/jpeg"},
                    {"js",    "application/javascript; charset=UTF-8"},
                    {"jsonp", "application/javascript; charset=UTF-8"},
                    {"json",  "application/json; charset=UTF-8"},
                    {"map",   "application/json; charset=UTF-8"},
                    {"gif",   "image/gif"},
                    {"css",   "text/css; charset=UTF-8"},
                    {"gz",    "application/x-gzip"},
                    {"gzip",  "multipart/x-gzip"},
                    {"ico",   "image/x-icon"},
                    {"png",   "image/png"},
                    {0,       0}
            };

    string cached_date_response = " Date: ";
    string api_cached_response = "\r\nConnection: close\r\nServer: pigeon\r\nAccept_Range: bytes\r\nContent-Type: application/json\r\n";

    const char *err_msg1 = "<!DOCTYPE html><html><head lang='en'><meta charset='UTF-8'><title>Status</title></head><body><table><th>Status Code</th><th>Message</th><tr><td>";
    const char *err_msg3 = "</td><td>";
    const char *err_msg5 = "</td></tr></table></body></html>";

namespace http_util {

    char *now() {

        time_t now = time(0);
        char *dt;

        tm *gmtm = gmtime(&now);
        dt = asctime(gmtm);
        dt[strlen(dt) - 1] = '\0';

        return dt;
    }

    string get_cached_response(bool is_api) {

        string cached_response;

        cached_response += cached_date_response;
        cached_response += now();

        if (is_api) {
            cached_response += api_cached_response;
        }
        return cached_response;

    }

    string get_header_field(HttpHeader hdr) {

        for (header *m = headers; m->header_id; ++m) {
            if (m->header_id == static_cast<int>(hdr)) {
                return m->header_field;
            }
        }

        return "unknown header";
    }

    string get_status_phrase(HttpStatus status) {

        for (statusphrase *m = statusphrases; m->status_code; ++m) {
            if (m->status_code == static_cast<int>(status)) {
                return m->status_phrase;
            }
        }

        return "unknown phrase";
    }

    string get_status_msg(HttpStatus status) {

        for (statusmsg *m = statusmsgs; m->status_code; ++m) {
            if (m->status_code == static_cast<int>(status)) {
                return m->status_msg;
            }
        }

        return "unknown msg";
    }

    string get_mime_type(string& extension) {

        for (mapping *m = mappings; m->extension; ++m) {
            if (m->extension == extension) {
                return m->mime_type;
            }
        }

        return string("text/plain");

    }

    string get_log_type(LogType type) {

        for (logtype *m = logtypes; m->log_id; ++m) {
            if (m->log_id == static_cast<int>(type)) {
                return m->log_type;
            }
        }

        return "unknown log type";
    }

    string get_severity(Severity severe) {
        for (severity *m = severities; m->severity_id; ++m) {
            if (m->severity_id == static_cast<int>(severe)) {
                return m->severity_type;
            }
        }

        return "unknown severity type";
    }

    bool is_api(string& Uri){
		const char* pos = strstr(Uri.c_str(), "/api/");
		if (pos){ return true; }
		else { return false; }
    }

    void parse_query_string(http_request& req){

        string query_uri(req.url);
        std::size_t _parStart = req.url.find('?');

        if (_parStart != string::npos) {

            req.url = query_uri.substr(0, _parStart);
            query_uri = query_uri.substr(_parStart + 1, query_uri.size());
        }

        if (_parStart != string::npos) {
            replace(query_uri.begin(), query_uri.end(), '&', ' ');
            std::istringstream issParams(query_uri.c_str());
            vector<string> vparams{ istream_iterator<string>{issParams},
                                    istream_iterator<string>{} };
            size_t end;
            for (auto &par : vparams) {
                end = par.find("=", 0);
                key_value_pair kvp;
                kvp.key = par.substr(0, end);
                kvp.value = par.substr(end + 1, par.size() - 1);
                req.set_parameter(kvp);
            }
        }
    }

    bool url_decode(const string& in, string& out) {

        out.clear();
        out.reserve(in.size());
        for (std::size_t i = 0; i < in.size(); ++i)
        {
            if (in[i] == '%')
            {
                if (i + 3 <= in.size())
                {
                    int value = 0;
                    std::istringstream is(in.substr(i + 1, 2));
                    if (is >> std::hex >> value)
                    {
                        out += static_cast<char>(value);
                        i += 2;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else if (in[i] == '+')
            {
                out += ' ';
            }
            else
            {
                out += in[i];
            }
        }
        return true;

    }

    void prepare(HttpStatus status, http_context *context) {

        context->response->message += get_status_phrase(status);
        context->response->message += get_cached_response(context->request->is_api);

    }

    void finish(HttpStatus status, http_context *context) {

        if (status != HttpStatus::OK && status != HttpStatus::NotModified) {
            context->response->message += "\r\n";
            context->response->message += err_msg1;
            context->response->message += std::to_string((int)status);
            context->response->message += err_msg3;
            context->response->message += http_util::get_status_msg(status);
            context->response->message += err_msg5;
        }
        else {
            context->response->get_non_default_headers(context->response->message);
            context->response->message += "\r\n";
            context->response->message += context->response->content;
        }

    }

    void process(http_context *context) {

        if(context->request->is_api){

            shared_ptr<http_handler_base> handler = rest_handlers::get()->get_handler(context->request->url);
            if (!handler)
            {
                prepare(HttpStatus::NotFound, context);
                return;
            }
            handler->process(context);
            prepare(HttpStatus::OK, context);
            {
                context->response->message += get_header_field(HttpHeader::Content_Length);
                context->response->message += std::to_string(context->response->content.size());
                context->response->message += "\r\n";
            }
            finish(HttpStatus::OK, context);

        } else {

            auto handler = std::make_shared<http_handler>();
            handler->process(context);

        }
    }


}


