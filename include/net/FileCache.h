//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_CACHE_H
#define PIGEON_CACHE_H

#include <uv.h>
#include <map>
#include <string>
#include <mutex>
#include "FileInfo.h"

using namespace std;

namespace pigeon {
    namespace net {
        class FileCache {
        private:
            string _path;
            map<string, FileInfo*> cache_data;
            uv_rwlock_t cache_lock;
            void CompressItem(FileInfo *fi);
            void CacheItem(string &file);
            void LoadFiles(string filepath, uv_fs_t *req);
        public:
            FileCache();
            ~FileCache();
            void Load(string path);
            void ReloadItem(string &file);
            FileInfo* GetItem(string &file_name);
        };
    }
}


#endif //PIGEON_CACHE_H
