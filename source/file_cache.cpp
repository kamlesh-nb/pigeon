#include "file_cache.h"
#include "app_constants.h"
#include "md5.h"
#include <sstream>
#include <iterator>
#include <zlib.h>
#include <fstream>
#include <algorithm>

using namespace pigeon;


void refresh(uv_fs_event_t *handle, const char *filename, int events, int status) {
    char path[2048];
    size_t size = 2047;

    uv_fs_event_getpath(handle, path, &size);
    path[size] = '\0';


    if (events & UV_CHANGE){
        string file(path);
        uv_fs_event_stop(handle);
        file_cache::get()->reload_item(file);
    }

}

file_cache::file_cache() {

}

file_cache::~file_cache() {

}

void file_cache::load(string path) {
    _path = path.append("/");

    load_files(_path,true);
}

void file_cache::load_files(string filepath, bool recursive) {

    DIR *dp;
    struct dirent *dirp;


    if ((dp = opendir(filepath.c_str())) == NULL) {
        return;
    } else {
        while ((dirp = readdir(dp)) != NULL) {
            if (dirp->d_name != string(".") && dirp->d_name != string("..")) {
                string srchPath = filepath + dirp->d_name;

                if (is_directory(srchPath)  && recursive ) {
                    load_files(filepath + dirp->d_name + "/", true);
                }
            }
        }
        closedir(dp);
    }

}

void file_cache::unload() {
	cache_data.clear();
}

bool file_cache::is_directory(string &file) {

	struct stat fileInfo;
	stat(file.c_str(), &fileInfo);
	bool result = false;

	if (S_ISDIR(fileInfo.st_mode)) {
		result = true;
	} else {
		cache_item(file);
		uv_fs_event_t* fs_event;
		fs_event = (uv_fs_event_t*)malloc(sizeof(uv_fs_event_t));
		uv_fs_event_init(uv_default_loop(), fs_event);
		uv_fs_event_start(fs_event, refresh, file.c_str(), 0);
		result =  false;
	}

	return result;
}

void file_cache::reload() {
	if (_path.size() > 0){
		cache_data.clear();

	}
}

void file_cache::reload_item(string &file) {

	auto cacheItem = std::find(cache_data.begin(),
		cache_data.end(),
		file_info(file));

	if (cacheItem != cache_data.end()){
		cache_data.erase(cacheItem);
	}

	cache_item(file);
	
	uv_fs_event_t* fs_event;
	fs_event = (uv_fs_event_t*)malloc(sizeof(uv_fs_event_t));
	uv_fs_event_init(uv_default_loop(), fs_event);
	uv_fs_event_start(fs_event, refresh, file.c_str(), 0);

}

void file_cache::get_item(string &file, file_info& fi) {

	for (auto& data : cache_data){
		if (data.file_name == fi.file_name){
			fi = data;
		}
	}

	/*auto cacheItem = std::find(cache_data.begin(),
		cache_data.end(),
		fi);

	if (cacheItem != cache_data.end()){
		fi = *cacheItem;
	}
	else {
		fi.file_size = 0;
	}*/

}

void file_cache::compress_item(file_info& fi) {

	string str(fi.content);
	z_stream zs;                        // z_stream is zlib's control structure
	memset(&zs, 0, sizeof(zs));
	int compressionlevel = Z_BEST_COMPRESSION;
	if (deflateInit(&zs, compressionlevel) != Z_OK)
		throw(std::runtime_error("deflateInit failed while compressing."));

	zs.next_in = (Bytef*)str.data();
	zs.avail_in = str.size();           // set the z_stream's input

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

void file_cache::cache_item(string &file) {

    struct stat file_stat;
    stat(file.c_str(), &file_stat);

	file_info fi(file);

	std::ifstream is;
	is.open(file.c_str(), std::ios::in | std::ios::binary);
	if (is)
	{

		std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

        time_t  lwt_t;
        char* lwt;

        lwt_t = (time_t)file_stat.st_mtim.tv_sec;

        lwt = ctime(&lwt_t);

        lwt[strlen(lwt) - 1] = '\0';


		fi.file_size = content.size();
		fi.last_write_time = lwt;

		string md5_src = file;
		md5_src += lwt;
		fi.etag = md5(md5_src);

        std::size_t last_slash_pos = file.find_last_of("/");
        std::size_t last_dot_pos = file.find_last_of(".");
        std::string extension;
        if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
        {
            extension = file.substr(last_dot_pos + 1);
        }



		fi.content = content;
		fi.file_name = file;

		///uncompressed headers
		fi.cached_headers += "\r\nConnection: close\r\nServer: pigeon\r\nAccept_Range: bytes\r\n";
		fi.cached_headers += app_constants::get_header_field(HttpHeader::Content_Type);
		fi.cached_headers += app_constants::get_mime_type(extension);
		fi.cached_headers += "\r\n";

		fi.cached_headers += app_constants::get_header_field(HttpHeader::Content_Length);
		fi.cached_headers += std::to_string(fi.file_size);
		fi.cached_headers += "\r\n";

		fi.cached_headers += app_constants::get_header_field(HttpHeader::Last_Modified);
		fi.cached_headers += fi.last_write_time;
		fi.cached_headers += "\r\n";

		fi.cached_headers += app_constants::get_header_field(HttpHeader::ETag);
		fi.cached_headers += fi.etag;
		fi.cached_headers += "\r\n";


        compress_item(fi);

		///compresses headers
		fi.compresses_cached_headers += "\r\nConnection: close\r\nServer: pigeon\r\nAccept_Range: bytes\r\n";
		fi.compresses_cached_headers += app_constants::get_header_field(HttpHeader::Content_Encoding);

		fi.compresses_cached_headers += app_constants::get_header_field(HttpHeader::Content_Type);
		fi.compresses_cached_headers += app_constants::get_mime_type(extension);
		fi.compresses_cached_headers += "\r\n";

		fi.compresses_cached_headers += app_constants::get_header_field(HttpHeader::Content_Length);
		fi.compresses_cached_headers += std::to_string(fi.compressed_file_size);
		fi.compresses_cached_headers += "\r\n";

		fi.compresses_cached_headers += app_constants::get_header_field(HttpHeader::Last_Modified);
		fi.compresses_cached_headers += fi.last_write_time;
		fi.compresses_cached_headers += "\r\n";

		fi.compresses_cached_headers += app_constants::get_header_field(HttpHeader::ETag);
		fi.compresses_cached_headers += fi.etag;
		fi.compresses_cached_headers += "\r\n";



		cache_data.push_back(std::move(fi));


	}

}


std::shared_ptr<file_cache> file_cache::instance = nullptr;

std::mutex file_cache::_mtx;

std::shared_ptr<file_cache>& file_cache::get()
{
	static std::shared_ptr<file_cache> tmp = instance;

	if (!tmp)
	{
		std::lock_guard<std::mutex> lock(_mtx);

		if (!tmp)
		{
			instance.reset(new file_cache());
			tmp = instance;
		}
	}
	return tmp;
}
