//
// Created by kamlesh on 9/12/16.
//

#ifndef PIGEON_RESULT_SET_H
#define PIGEON_RESULT_SET_H

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include "rapidjson/writer.h"

using namespace std;
using namespace rapidjson;

namespace pigeon {
    namespace data {
        namespace rdb {
            class ResultSet {
            private:
                unsigned int response_type;
                string json;
                Value::ConstValueIterator row;
                unsigned int cur_row_index;
                SizeType size;
            public:
                ResultSet();
                ~ResultSet();
                void ParseResult(char*);
                bool HasRows();
                uint GetUint(string);
                int GetInt(string);
                uint64_t GetUint64(string);
                int64_t GetInt64(string);
                double GetDouble(string);
                string GetString(string);
                bool GetBool(string);
                float GetFloat(string);
                string ToJson();
            };
        }
    }
}

#endif //PIGEON_RESULT_SET_H
