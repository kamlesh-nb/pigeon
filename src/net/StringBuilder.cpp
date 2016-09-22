
#include "StringBuilder.h"

using namespace pigeon::net;


StringBuilder::StringBuilder() {
    Reserve(1024);
    cur_pos = 0;
    length = 1024;
}

StringBuilder::~StringBuilder() {

}

void StringBuilder::Reset() {
    cur_pos = 0;
    memset(base, '\0', length);
}

void StringBuilder::Clear() {
    free(base);
}

void StringBuilder::resize(size_t sz) {

    char* old_base = base;

    base = (char*)realloc(base, sz);

    if (base == NULL) {
        base = old_base;
        return;
    }

    memset(base + cur_pos, '\0', sz - cur_pos);
    length = sz;

}

char* StringBuilder::Find(char *str) {
    char * pch;
    pch = strstr (base, str);
	return pch;
}

size_t StringBuilder::GetLength() {
	return cur_pos;
}

void StringBuilder::Reserve(size_t capacity) {
    base = (char*)malloc(capacity);
}

void StringBuilder::Append(char *str) {

    size_t space_remaining;
    size_t space_required;
    size_t new_length;
    size_t size = strlen(str);

    space_remaining = length - cur_pos - 1;
    if (space_remaining < size)  {
        space_required = size - space_remaining;
        new_length = length;
        do {
            new_length = new_length * 2;
        } while (new_length < (length + space_required));

        resize(new_length);
    }

    memcpy(base + cur_pos, str, size);
    cur_pos += size;

}

void StringBuilder::Append(char *str, unsigned long length) {
    size_t space_remaining;
    size_t space_required;
    size_t new_length;
    size_t size = length;

    space_remaining = length - cur_pos - 1;
    if (space_remaining < size)  {
        space_required = size - space_remaining;
        new_length = length;
        do {
            new_length = new_length * 2;
        } while (new_length < (length + space_required));

        resize(new_length);
    }

    memcpy(base + cur_pos, str, size);
    cur_pos += size;
}

char *StringBuilder::ToCStr() {
    return base;
}