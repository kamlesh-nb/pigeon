#ifndef PIGEON_HTTP_MSG_H
#define PIGEON_HTTP_MSG_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include "FileInfo.h"
#include "HttpUtil.h"
#include "StringBuilder.h"

using namespace std;

namespace pigeon {

    namespace net {
        class HttpMessage {
        private:
            unordered_map<string, string> headers;
            string temp;
        protected:
            unordered_map<string, string> cookies;
            auto AcceptsDeflated() -> bool;
        public:
            virtual ~HttpMessage();
            int http_major_version;
            int http_minor_version;
            string content;
            StringBuilder *buffer;
            auto HasCookies() -> bool;
            virtual auto SetHeader(string &, string &) -> void;
            virtual auto GetHeader(string) -> string &;
            virtual auto SetCookie(string &, string &) -> void;
            virtual auto GetNonDefaultHeaders() -> void;
        };
        struct Form {
            unordered_map<string, string> headers;
            unordered_map<string, string> parameters;
            string file_data;
        };
        class HttpResponse : public HttpMessage {
        public:
            virtual ~HttpResponse();
        };
        class HttpRequest : public HttpMessage {
        private:
            unordered_map<string, string> parameters;
        public:
            virtual ~HttpRequest();
            string url;
            unsigned int method;
            bool is_api{false};
            vector<Form> forms;
            auto GetCookie(string) -> string &;
            auto GetParameter(string &) -> string &;
            auto SetParameter(string &, string &) -> void;
            auto CreateResponse(const char *, HttpResponse *response, HttpStatus status) -> void;
            auto CreateResponse(string &, HttpResponse *response, HttpStatus status, bool deflate) -> void;
            auto CreateResponse(FileInfo &, HttpResponse *response, HttpStatus status) -> void;
        };
    }

}
#endif //PIGEON_HTTP_MSG_H