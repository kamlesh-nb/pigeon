//
// Created by kamlesh on 25/3/16.
//

#ifndef PIGEON_MULTI_PART_PARSER_H
#define PIGEON_MULTI_PART_PARSER_H

#include "HttpContext.h"

namespace pigeon {
    namespace net {
        class MultiPartParser {
        private:
            vector<string> file_contents;
            string boundary;

            Form ParsePart(string, string &boundary);

        public:
            void Parse(HttpContext *, string);
        };
    }
}


#endif //PIGEON_MULTI_PART_PARSER_H
