#include "http_msg.h"

using namespace pigeon;

http_msg::~http_msg() {

}

auto http_msg::set_header_field(string& _key) -> void{

    key_value_pair kvp;
    kvp.key = _key;
    headers.push_back(kvp);

}

auto http_msg::set_header_value(string& _value) -> void{

    if(headers.empty()) {
        return;
    }
    headers.back().value = _value;

}

auto http_msg::get_header(key_value_pair& kvp) -> void {

    for(auto& hdr : headers){
        if(hdr.key == kvp.key){
            kvp = hdr;
        }
    }

}

auto http_msg::get_non_default_headers(string &msg) -> void {

    for (auto& hdr : headers){
        msg += hdr.key;
        msg += hdr.value;
        msg += "\r\n";
    }

}

auto http_request::get_parameter(key_value_pair& kvp) -> void {

    for(auto& prm : parameters){
        if(prm.key == kvp.key){
            kvp = prm;
        }
    }

}

auto http_request::set_parameter(key_value_pair& kvp) -> void {
    parameters.push_back(std::move(kvp));
}

http_response::~http_response() {

}

http_request::~http_request() {

}