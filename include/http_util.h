#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#include <string>
#include "http_context.h"
#include "http_handler_base.h"


using namespace std;
using namespace pigeon;


namespace http_util {

    enum class LogType {
        Warning = 1, Information, Error
    };
    enum class Severity {
        Low = 1, Medium, High, Critical
    };
    enum class HttpHeader {
        Content_Type = 1,
        Content_Length,
        Last_Modified,
        ETag,
        Content_Encoding
    };
    enum class HttpStatus {

        Continue = 100,
        SwitchingProtocols = 101,
        Checkpoint = 103,
        OK = 200,
        Created = 201,
        Accepted = 202,
        NonAuthoritativeInformation = 203,
        NoContent = 204,
        ResetContent = 205,
        PartialContent = 206,
        MultipleChoices = 300,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        SwitchProxy = 306,
        TemporaryRedirect = 307,
        ResumeIncomplete = 308,
        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        NotAcceptable = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout = 408,
        Conflict = 409,
        Gone = 410,
        LengthRequired = 411,
        PreconditionFailed = 412,
        RequestEntityTooLarge = 413,
        RequestURITooLong = 414,
        UnsupportedMediaType = 415,
        RequestedRangeNotSatisfiable = 416,
        ExpectationFailed = 417,
        InternalTcpServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HTTPVersionNotSupported = 505,
        NetworkAuthenticationRequired = 511

    };



    string get_cached_response(bool is_api);
    char *now();
    string get_header_field(HttpHeader hdr);
    string get_status_phrase(HttpStatus status);
    string get_status_msg(HttpStatus status);
    string get_mime_type(string &extension);
    string get_log_type(LogType type);
    string get_severity(Severity severe);
    bool is_api(string& Uri);
    void parse_query_string(http_request & req);
    bool url_decode(const string &in, string &out);
    void prepare(HttpStatus status, http_context *context);
    void finish(HttpStatus status, http_context *context);
    void process(http_context*);

}





#endif //HTTP_UTIL_H

