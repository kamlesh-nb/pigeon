//
// Created by kamlesh on 25/3/16.
//

#ifndef PIGEON_MULTI_PART_PARSER_H
#define PIGEON_MULTI_PART_PARSER_H

#include "http_context.h"

namespace pigeon {


    class multi_part_parser {

    private:
        bool is_char(int c);
        bool is_ctl(int c);
        bool is_tspecial(int c);

        enum state
        {
            boundary_start,
            expecting_newline_1,
            header_line_start,
            header_lws,
            header_name,
            space_before_header_value,
            header_value,
            expecting_newline_2,
            expecting_newline_3,
            body_start,
            body_end
        } state_;

        string part_data;
        string::iterator begin;
        string::iterator end;

    public:
        string parse_body(string::iterator start, string::iterator finish, string& boundary);
        void execute_parser(http_context*);


    };

}


#endif //PIGEON_MULTI_PART_PARSER_H
