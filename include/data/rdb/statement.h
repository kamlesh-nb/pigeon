//
// Created by kamlesh on 9/12/16.
//

#ifndef PIGEON_QUERY_H
#define PIGEON_QUERY_H


#include <stack>
#include <string>
#include "data/rdb/result_set.h"
#include "rdb_connection.h"

using namespace std;

namespace pigeon {
    namespace data {
        namespace rdb {
            class statement {
            private:
                stack<string> child;
                string msg;
                string dbname;
                rdb_connection* mConnection;
            public:

                statement(rdb_connection*);
                ~statement();
                statement* table(string tablename);
                statement* filter(string filter_criteria);
                statement* insert(string data);
                statement* get(string data);
                statement* getAll(string data);
                statement* eqJoin(string data);
                statement* outerJoin(string data);
                statement* innerJoin(string data) ;
                statement* zip(string data);
                statement* group(string data);
                statement* orderBy(string data);
                statement* db(string name);
                void run();
            };
        }
    }

}


#endif //PIGEON_QUERY_H
