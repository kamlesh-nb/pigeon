#ifndef KEY_VALUE_PAIR_H
#define KEY_VALUE_PAIR_H

#include <string>

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

    struct key_value_pair {
        std::string key;
        std::string value;
    };
}

#endif //KEY_VALUE_PAIR_H