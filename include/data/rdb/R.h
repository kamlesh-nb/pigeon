//
// Created by kamlesh on 9/22/16.
//

#ifndef PIGEON_R_H
#define PIGEON_R_H

#include <stack>
#include <functional>
#include "net/HttpContext.h"
#include "data/rdb/ResultSet.h"

using namespace pigeon::net;

namespace pigeon {
    namespace data {
        namespace rdb {
            class R {
            private:
                std::stack<string> child;
                string msg;
                string dbname;
            public:
                R();
                ~R();
                R db(string name);
                R table(string tablename);
                R filter(string filter_criteria);
                R insert(string data);
                R get(string data);
                R getAll(string data);
                R eqJoin(string data);
                R outerJoin(string data);
                R innerJoin(string data);
                R zip(string data);
                R group(string data);
                R orderBy(string data);
                void run(HttpContext*, std::function<void(HttpContext*, ResultSet&)>);
            };
        }
    }
}


#endif //PIGEON_R_H
