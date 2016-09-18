//
// Created by kamlesh on 9/12/16.
//

#ifndef PIGEON_QUERY_H
#define PIGEON_QUERY_H


#include <stack>
#include <string>
#include <functional>
#include "data/rdb/result_set.h"
#include "data/db_connection.h"

#include "http_context.h"
using namespace pigeon::net;

using namespace std;

namespace pigeon {
    namespace data {
        namespace rdb {
            typedef std::function<void(http_context*, result_set*)> request_cb;
            class R {
            private:
                stack<string> child;
                string msg;
                string dbname;
                http_context* m_http_context;
                request_cb requestCb;
                result_set* parse_result(char*);
            public:
                R(http_context*);
                ~R();
                R* table(string tablename);
                R* filter(string filter_criteria);
                R* insert(string data);
                R* get(string data);
                R* getAll(string data);
                R* eqJoin(string data);
                R* outerJoin(string data);
                R* innerJoin(string data) ;
                R* zip(string data);
                R* group(string data);
                R* orderBy(string data);
                R* db(string name);

                void on_query_sent(uv_write_t *req, int status);
                void on_fetch_result(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
                void run(db_connection*, std::function<void(http_context*, result_set*)>);
            };
        }

    }

}


#endif //PIGEON_QUERY_H
