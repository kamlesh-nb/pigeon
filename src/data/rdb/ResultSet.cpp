//
// Created by kamlesh on 9/12/16.
//

#include "data/rdb/ResultSet.h"

using namespace pigeon::data::rdb;



ResultSet::ResultSet(char* data) {

    Document doc;
    char* begin = data;
    if(begin){
        while (true){
            if(*begin == '{'){
                break;
            } else { ++begin; }
        }

     doc.Parse<kParseStopWhenDoneFlag>(begin);
    }

    const Value& res = doc["r"];
    if(res.IsArray()){
        size = res.Capacity();
        row = res.Begin();
        cur_row_index = 0;
    }

    StringBuffer buffer;
    buffer.Clear();
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    json = buffer.GetString();
}

ResultSet::~ResultSet() {
}

bool ResultSet::HasRows() {
    ++cur_row_index;
    if(cur_row_index > 1){
        row++;
    }
    return size >= cur_row_index;
}

uint ResultSet::GetUint(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetUint();
}

int ResultSet::GetInt(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetInt();
}

uint64_t ResultSet::GetUint64(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetUint64();
}

int64_t ResultSet::GetInt64(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetInt64();
}

double ResultSet::GetDouble(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetDouble();
}

string ResultSet::GetString(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetString();
}


bool ResultSet::GetBool(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetBool();
}

float ResultSet::GetFloat(string fieldName) {
    auto data = row->FindMember(fieldName.c_str());
    return data->value.GetFloat();
}


string ResultSet::ToJson() {

    return json;
}





