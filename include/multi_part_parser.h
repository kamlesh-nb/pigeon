//
// Created by kamlesh on 25/3/16.
//

#ifndef PIGEON_MULTI_PART_PARSER_H
#define PIGEON_MULTI_PART_PARSER_H

#include "http_context.h"

namespace pigeon {


    class multi_part_parser {

    private:

        vector<string> file_contents;
        string boundary;

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

        enum param_state
        {
            param_start,
            param_name,
            param_value
        } param_state_;


    public:

        form parse_part(string, string& boundary);
        void parse(http_context*, string);

    };

}


#endif //PIGEON_MULTI_PART_PARSER_H
