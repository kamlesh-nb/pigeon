//
// Created by kamlesh on 25/3/16.
//

#ifndef PIGEON_MULTI_PART_PARSER_H
#define PIGEON_MULTI_PART_PARSER_H

#include <http_context.h>

namespace pigeon {


    class multi_part_parser {

    private:

        vector<string> file_contents;
        string boundary;




    public:

        form parse_part(string, string &boundary);

        void parse(http_context *, string);

    };

}


#endif //PIGEON_MULTI_PART_PARSER_H
