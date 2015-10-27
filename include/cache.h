//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_CACHE_H
#define PIGEON_CACHE_H

#include <uv.h>
#include "file_info.h"
#include <vector>
#include <string>

using namespace std;

namespace pigeon {

    class cache {

        string _path;
        char *_buffer;
        vector<file_info> cache_data;

        uv_rwlock_t cache_lock;

        void compress_item(file_info& fi);

        void cache_item(string& file);

        void load_files(string filepath, uv_fs_t* req);

    public:

        cache();

        ~cache();

        void load(string path);

        void reload_item(string &file);

        void get_item(string &file, file_info& fi);

    };


}




#endif //PIGEON_CACHE_H
