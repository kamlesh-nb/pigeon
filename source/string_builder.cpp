
#include <string_builder.h>

using namespace pigeon;


string_builder::string_builder() {
    reserve(8192);
    cur_pos = 0;
    length = 8192;
}

string_builder::~string_builder() {

}


void string_builder::reset() {
    cur_pos = 0;
    memset(base, '\0', length);
}


void string_builder::clear() {
    free(base);
}

void string_builder::resize(size_t sz) {

    char* old_base = base;

    base = (char*)realloc(base, sz);

    if (base == NULL) {
        base = old_base;
        return;
    }

    memset(base + cur_pos, '\0', sz - cur_pos);
    length = sz;

}


char* string_builder::find(char * str) {
    char * pch;
    pch = strstr (base, str);
	return pch;
}

size_t string_builder::get_length() {
	return cur_pos;
}

void string_builder::reserve(size_t capacity) {
    base = (char*)malloc(capacity);
}

void string_builder::append(char *str) {

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


void string_builder::append(char *str, unsigned long length) {
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


char *string_builder::to_cstr() {
    return base;
}