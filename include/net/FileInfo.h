//
// Created by kamlesh on 9/5/16.
//

#ifndef PIGEON_FILE_INFO_H
#define PIGEON_FILE_INFO_H

#include <string>

namespace pigeon {

    namespace net {
        struct FileInfo {
            std::string file_name;
            std::string cached_headers;
            std::string compresses_cached_headers;
            std::string etag;
            size_t file_size = 0;
            size_t compressed_file_size;
            std::string last_write_time;
            std::string content;
            std::string compressed_content;

            FileInfo(std::string &fileName) {
                file_name = fileName;
            }

            friend bool operator==(const FileInfo &fi1, const FileInfo &fi2) {
                return fi1.file_name == fi2.file_name;
            }

        };
    }

}

#endif //PIGEON_FILE_INFO_H
