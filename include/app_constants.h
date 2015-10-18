#ifndef HTTP_CONSTANTS_H
#define HTTP_CONSTANTS_H

#include <string>

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

	class app_constants {

	public:

		static string get_cached_response(bool);

		static string get_header_field(HttpHeader);

		static string get_status_phrase(HttpStatus);

		static string get_status_msg(HttpStatus);

		static string get_mime_type(string &);

		static string get_log_type(LogType);

		static string get_severity(Severity);

		static char *now();

		app_constants();

		~app_constants();
	};
}
#endif //HTTP_CONSTANTS_H

