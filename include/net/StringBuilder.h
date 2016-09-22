//
// Created by kamlesh on 30/4/16.
//

#ifndef PIGEON_STRING_BUILDER_H
#define PIGEON_STRING_BUILDER_H

#include <cstring>
#include <malloc.h>

namespace pigeon {

    namespace net {

    class StringBuilder {

    private:
        char* base;
        size_t length;
        size_t cur_pos;
        void resize(size_t);
    public:
        StringBuilder();
        ~StringBuilder();
        char* Find(char *);
        size_t GetLength();
        void Reserve(size_t);
        void Reset();
        void Append(char *);
        void Append(char *, unsigned long);
        char* ToCStr();
        void Clear();

    };

}

}

#endif //PIGEON_STRING_BUILDER_H