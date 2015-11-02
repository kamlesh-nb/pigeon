#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <string>

namespace pigeon {

	struct file_info {
		std::string file_name;
		std::string cached_headers;
		std::string compresses_cached_headers;
		std::string etag;
		size_t file_size = 0;
		size_t compressed_file_size;
		std::string last_write_time;
		std::string content;
		std::string compressed_content;

		file_info(std::string &fileName) {
			file_name = fileName;
		}

		friend bool operator==(const file_info &fi1, const file_info &fi2) {
			return fi1.file_name == fi2.file_name;
		}

	};

}

#endif //FILE_INFO_H