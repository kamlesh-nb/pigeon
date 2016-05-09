//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_CACHE_H
#define PIGEON_CACHE_H

#include <uv.h>
#include <vector>
#include <string>
#include <mutex>
#include "file_info.h"

using namespace std;

namespace pigeon {

    class cache {

    private:
        static std::mutex _mtx;
        static std::shared_ptr<cache> temp;

        string _path;

        vector<file_info> cache_data;

        uv_rwlock_t cache_lock;

        void compress_item(file_info &fi);

        void cache_item(string &file);

        void load_files(string filepath, uv_fs_t *req);

    public:

        cache();

        ~cache();

        void load(string path);

        void reload_item(string &file);

        void get_item(file_info &fi);

        static std::shared_ptr<cache> &get();

    };


}


#endif //PIGEON_CACHE_H
