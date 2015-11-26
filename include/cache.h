//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_CACHE_H
#define PIGEON_CACHE_H

#include "file_info.h"
#include <vector>
#include <string>
#include <mutex>

using namespace std;

namespace pigeon {

    class cache {

	private:

        string _path;
        char *_buffer;
        vector<file_info> cache_data;
		std::mutex _get_item_mtx;
		static std::mutex _mtx;
		static std::shared_ptr<cache> instance;

        void compress_item(file_info& fi);
        void cache_item(string& file);
        void load_files(string filepath);
        void path_correction(string&);
        

    public:

        cache();

        ~cache();

        void load(string path);

        void reload_item(string &file);

        void get_item(string &file, file_info& fi);

		static std::shared_ptr<cache>& get();

    };


}




#endif //PIGEON_CACHE_H
