#ifndef FILE_CACHE_H
#define FILE_CACHE_H


#include <memory>
#include <mutex>
#include <vector>
#include <uv.h>
#include "file_info.h"

using namespace std;

namespace pigeon {

	class file_cache {

	private:

	private:


		string _path;
		char *_buffer;
		vector<file_info> cache_data;
		static std::mutex _mtx;
		static std::shared_ptr<file_cache> instance;

		bool is_directory(string &);

		void compress_item(file_info &);

		void cache_item(string&);

		void load_files(string, bool);

	public:

		file_cache();

		~file_cache();

		void load(string);

		void unload();

		void reload();

		void reload_item(string&);

		void get_item(string &, file_info &);

		static std::shared_ptr<file_cache> &get();

	};

}
#endif //FILE_CACHE_H
