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
        void load_file_data(http_context*);

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
            param_separater,
            ke_val_separater
        } param_state_;

        string part_data;
        string::iterator begin;
        string::iterator end;

    public:

        form parse(string, string& boundary);
        void parse_multipart(http_context*, string);


    };

}


#endif //PIGEON_MULTI_PART_PARSER_H
