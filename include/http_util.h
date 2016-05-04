#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#include <string>
#include <vector>
#include <string_builder.h>

using namespace std;

namespace pigeon {

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
        Content_Encoding,
        Set_Cookie,
        Authorization,
        Access_Control_Allow_Origin,
        Access_Control_Allow_Methods,
        Access_Control_Allow_Headers,
        Access_Control_Max_Age,
        Access_Control_Allow_Credentials,
        Access_Control_Expose_Headers,
        Vary,
        Keep_Alive,
        Connection
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

    auto get_cached_response(bool is_api, string_builder*) -> void;

    char *now();

    auto get_header_field(HttpHeader hdr) -> const char*;

    auto get_header_field(HttpHeader hdr, string&) -> void;

    auto get_status_phrase(HttpStatus status) -> const char*;

	auto get_status_msg(HttpStatus status) -> const char*;

	auto get_err_msg(const char*, HttpStatus status, string_builder*) -> void;

	auto get_mime_type(string &extension) -> const string;

	auto get_log_type(LogType type) -> const string;

	auto get_severity(Severity severe) -> const string;

	auto url_decode(const string &in, string &out) -> bool;

	auto deflate_string(string&, string&) -> unsigned long;

}


#endif //HTTP_UTIL_H

