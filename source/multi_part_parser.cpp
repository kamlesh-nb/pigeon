//
// Created by kamlesh on 25/3/16.
//


#include <iterator>
#include <sstream>
#include <fstream>
#include <settings.h>
#include <algorithm>
#include <multi_part_parser.h>
#include <regex>

using namespace pigeon;



void multi_part_parser::load_file_data(http_context *) {

}


form multi_part_parser::parse(string data, string &boundary) {
    state_ = header_line_start;
    form form_data;
    string temp;
    string val;

    string::iterator begin = data.begin();
    string::iterator end = data.end();


    char c;
    while(begin != end) {

        if(state_ != body_start){
            c = *begin++;
        }

        switch (state_) {

            case header_line_start:
                if (c == '\r')
                {
                    state_ = expecting_newline_3;
                } else if (!form_data.headers.empty() && (c == ' ' || c == '\t'))
                {
                    state_ = header_lws;
                }
                else
                {
                    temp.push_back(c);
                    state_ = header_name;
                }
                break;

            case header_name:
                if (c == ':')
                {
                    state_ = space_before_header_value;
                }
                else
                {
                    temp.push_back(c);
                }
                break;

            case space_before_header_value:
                if (c == ' ')
                {
                    state_ = header_value;
                }
                break;
            case header_value:
                if (c == '\r')
                {
                    form_data.headers.emplace(std::pair<string, string>(temp, val));
                    temp.clear();
                    val.clear();
                    state_ = expecting_newline_2;
                }
                else
                {
                    val.push_back(c);
                }
                break;
            case expecting_newline_2:
                if (c == '\n')
                {
                    state_ = header_line_start;
                }
                break;
            case expecting_newline_3:
                if(c == '\n')
                {
                    state_ = body_start;
                }

                break;
            case body_start:
                string body_with_boundary = string(begin, end);
                long pos = body_with_boundary.find(boundary);
                string body_without_boundary = body_with_boundary.substr(0, pos-2);
                form_data.file_data = body_without_boundary;

                begin = end;
                break;

        }
    }

    string cont_disp = form_data.headers["Content-Disposition"];

   if(cont_disp.size() > 0){
       param_state_ = param_start;

       for(auto& c:cont_disp){

       }

   }

    return  form_data;
}




void multi_part_parser::parse_multipart(http_context* context, string _boundary) {

    boundary = _boundary;

    string parts = context->request->content;

    unsigned long start_pos = parts.find(boundary);
    while( start_pos != string::npos){
        unsigned long end_pos = parts.find(boundary, start_pos + boundary.size());
        if(end_pos != string::npos)
        {
            unsigned long start_from = start_pos + boundary.size() + 2;
            string part = parts.substr(start_from, end_pos-2);
            file_contents.push_back(part);
        }
        start_pos = parts.find(boundary, end_pos);
    }

    for(auto& str: file_contents){
      context->request->forms.push_back(parse(str, boundary));
    }

}


