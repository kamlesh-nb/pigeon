//
// Created by kamlesh on 25/3/16.
//


#include <iterator>
#include <sstream>
#include <fstream>
#include <settings.h>
#include <algorithm>
#include <multi_part_parser.h>

using namespace pigeon;


bool multi_part_parser::is_char(int c) {
    return c >= 0 && c <= 127;
}

bool multi_part_parser::is_ctl(int c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

bool multi_part_parser::is_tspecial(int c) {
    switch (c)
    {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
            return true;
        default:
            return false;
    }
}

string multi_part_parser::parse_body(string::iterator start, string::iterator finish, string& boundary){
    string body_with_boundary = string(begin, end);
    long pos = body_with_boundary.find(boundary);
    string body_without_boundary = body_with_boundary.substr(0, pos);
    return body_without_boundary;
}

void multi_part_parser::execute_parser(http_context* context) {
    state_ = boundary_start;
    form form_data;
    string temp;
    string val;
    part_data = context->request->content;
    begin = part_data.begin();
    end = part_data.end();

    while(begin != end) {
        char c = *begin++;
        switch (state_) {
            case boundary_start:
                if(c == '\r') {
                    context->request->form_data.boundary = temp;
                    temp.clear();
                    state_ = expecting_newline_1;
                } else {
                    temp.push_back(c);
                }

                break;
            case expecting_newline_1:
                if (c == '\n')
                {
                    state_ = header_line_start;
                }
                break;
            case header_line_start:
                if (c == '\r')
                {
                    state_ = expecting_newline_3;
                } else if (!context->request->form_data.headers.empty() && (c == ' ' || c == '\t'))
                {
                    state_ = header_lws;
                }
                else if (!is_char(c) || is_ctl(c) || is_tspecial(c))
                {
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
                else if (!is_char(c) || is_ctl(c) || is_tspecial(c))
                {

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
                    context->request->form_data.headers.emplace(std::pair<string, string>(temp, val));
                    temp.clear();
                    val.clear();
                    state_ = expecting_newline_2;
                }
                else if (is_ctl(c))
                {
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
                context->request->form_data.filedata =  parse_body(begin, end, context->request->form_data.boundary);
                begin = end;
                break;

        }
    }

    //parse parameters

    string content_disp_val = context->request->form_data.headers["Content-Disposition"];

    if(content_disp_val.size() > 0){
        replace(content_disp_val.begin(), content_disp_val.end(), ';', ' ');
        std::istringstream issParams(content_disp_val.c_str());
        vector<string> vparams{istream_iterator<string>{issParams},
                               istream_iterator<string>{}};

        for(auto& val:vparams){

            long end = val.find("=");
            if(end != string::npos){
                string name = val.substr(0, end);
                string value = val.substr(end+2, val.size() -2 );
                value.pop_back();
                context->request->form_data.parameters.emplace(std::pair<string, string>(name, value));
            }

        }



    }

}


