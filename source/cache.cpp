//
// Created by kamlesh on 23/10/15.
//

#include <fstream>
#include <string.h>
#include <md5.h>

#include <sstream>
#include <boost/filesystem.hpp>
#include <zlib.h>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <logger.h>

#include "http_util.h"
#include "cache.h"

using namespace pigeon;
namespace bfs = boost::filesystem;
 

cache::cache(){
    
}

cache::~cache(){}

void cache::path_correction(string& resource_path){

    for (auto& c : resource_path){
        if (c == '\\'){
            c = '/';
        }

    }
}

void cache::compress_item(file_info& fi){

    string str(fi.content);
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));
    int compressionlevel = Z_BEST_COMPRESSION;
    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw(std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = (Bytef*)str.data();
    zs.avail_in = static_cast<unsigned int>(str.size());           // set the z_stream's input

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            // append the block to the output string
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw(std::runtime_error(oss.str()));
    }

    zs.avail_out = zs.total_out;// outstring.size();
    fi.compressed_file_size = zs.total_out;// outstring.size();
    fi.compressed_content = outstring;

}

void cache::cache_item(string& file){
    

    file_info fi(file);

    std::ifstream is;
    is.open(file.c_str(), std::ios::in | std::ios::binary);
    if (is)
    {


        std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        bfs::path path(file);
        time_t lwt_t = bfs::last_write_time(path);
        char* lwt = ctime(&lwt_t);
        lwt[strlen(lwt) - 1] = '\0';

        fi.file_size = bfs::file_size(path);
        fi.last_write_time = lwt;
 
        string md5_src = file;
        md5_src += lwt;
        fi.etag = md5(md5_src); 

         
        string extn(path.extension().string());

        fi.content = content;
        fi.file_name = file;

        fi.content = content;
        fi.file_name = file;

        ///uncompressed headers
        fi.cached_headers += "\r\nCache-Control: public, max-age=0\r\nConnection: keep-alive\r\nServer: pigeon\r\nAccept_Range: bytes\r\n";
        fi.cached_headers += http_util::get_header_field(HttpHeader::Content_Type);
        fi.cached_headers += http_util::get_mime_type(extn);
        fi.cached_headers += "\r\n";

        fi.cached_headers += http_util::get_header_field(HttpHeader::Content_Length);
        fi.cached_headers += std::to_string(fi.file_size);
        fi.cached_headers += "\r\n";

        fi.cached_headers += http_util::get_header_field(HttpHeader::Last_Modified);
        fi.cached_headers += fi.last_write_time;
        fi.cached_headers += "\r\n";

        fi.cached_headers += http_util::get_header_field(HttpHeader::ETag);
        fi.cached_headers += fi.etag;
        fi.cached_headers += "\r\n";


        compress_item(fi);

        ///compresses headers
        fi.compresses_cached_headers += "\r\nCache-Control: public, max-age=0\r\nConnection: keep-alive\r\nServer: pigeon\r\nAccept_Range: bytes\r\n";
        fi.compresses_cached_headers += http_util::get_header_field(HttpHeader::Content_Encoding);

        fi.compresses_cached_headers += http_util::get_header_field(HttpHeader::Content_Type);
        fi.compresses_cached_headers += http_util::get_mime_type(extn);
        fi.compresses_cached_headers += "\r\n";

        fi.compresses_cached_headers += http_util::get_header_field(HttpHeader::Content_Length);
        fi.compresses_cached_headers += std::to_string(fi.compressed_file_size);
        fi.compresses_cached_headers += "\r\n";

        fi.compresses_cached_headers += http_util::get_header_field(HttpHeader::Last_Modified);
        fi.compresses_cached_headers += fi.last_write_time;
        fi.compresses_cached_headers += "\r\n";

        fi.compresses_cached_headers += http_util::get_header_field(HttpHeader::ETag);
        fi.compresses_cached_headers += fi.etag;
        fi.compresses_cached_headers += "\r\n";



        cache_data.push_back(std::move(fi));


    }
}

void cache::load_files(string filepath) {

    bfs::path path(filepath);

    bfs::recursive_directory_iterator itr(path);
    while (itr != bfs::recursive_directory_iterator())
    {
        if (!is_directory(itr->path()))    {
            string s = itr->path().string();
            path_correction(s);
            cache_item(s);
        }
        ++itr;
    }
}

void cache::load(string path){

    
    load_files(path);

}

void cache::reload_item(string &file) {

    auto cacheItem = std::find(cache_data.begin(),
        cache_data.end(),
        file_info(file));

    if (cacheItem != cache_data.end()){
        cache_data.erase(cacheItem);
    }

    cache_item(file);

}

void cache::get_item(string &file, file_info& fi) {

    path_correction(fi.file_name);

    for (auto& data : cache_data){
        if (data.file_name == fi.file_name){
            fi = data;
        }
    }
   
}

