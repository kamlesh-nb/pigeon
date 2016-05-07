//
// Created by kamlesh on 30/4/16.
//

#ifndef PIGEON_STRING_BUILDER_H
#define PIGEON_STRING_BUILDER_H

#include <cstring>
#include <malloc.h>

namespace pigeon {

    class string_builder {

    private:
        char* base;
        size_t length;
        size_t cur_pos;
        void resize(size_t);
    public:
        string_builder();
        ~string_builder();
        char* find(char*);
		size_t get_length();
        void reserve(size_t);
        void reset();
        void append(char*);
        void append(char*, unsigned long);
        char* to_cstr();
        void clear();

    };
}

#endif //PIGEON_STRING_BUILDER_H