//
// Created by kamlesh on 23/10/15.
//
#include <fstream>
#include <string.h>
#include <md5.h>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include "Logger.h"
#include "HttpUtil.h"
#include "FileCache.h"
#include <iostream>

using namespace pigeon;
using namespace pigeon::net;


void refresh(uv_fs_event_t *handle, const char *filename, int events, int status) {

    if (status != 0) {
        Logger::Get()->Write(LogType::Error, Severity::Critical, uv_err_name(status));
    }

    Logger::Get()->Write(LogType::Error, Severity::Critical, filename);
    
    char path[2048];
    size_t size = 2048;

    uv_fs_event_getpath(handle, path, &size);
    path[size] = '\0';


    if (events & UV_CHANGE) {

        string file(path);

        FileCache *rc = static_cast<FileCache *>(handle->data);
        rc->ReloadItem(file);
        uv_fs_event_stop(handle);

    }

}

FileCache::FileCache() {
    int r = uv_rwlock_init(&cache_lock);
    if (r != 0) {
        Logger::Get()->Write(LogType::Error, Severity::Critical, uv_err_name(r));
    }
}

FileCache::~FileCache() { }

void FileCache::CompressItem(FileInfo *fi) {

    string str(fi->content);
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));
    int compressionlevel = Z_BEST_COMPRESSION;
    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw (std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = (Bytef *) str.data();
    zs.avail_in = static_cast<unsigned int>(str.size());           // set the z_stream's input

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
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
        throw (std::runtime_error(oss.str()));
    }

    zs.avail_out = zs.total_out;// outstring.size();
    fi->compressed_file_size = zs.total_out;// outstring.size();
    fi->compressed_content = outstring;

}

void FileCache::CacheItem(string &file) {
    struct stat file_stat;
    stat(file.c_str(), &file_stat);

    FileInfo* fi = new FileInfo(file);

    std::ifstream is;
    is.open(file.c_str(), std::ios::in | std::ios::binary);
    if (is) {

        std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

        time_t lwt_t;
        char *lwt;
        uv_fs_t req;
        int r = uv_fs_stat(uv_default_loop(), &req, file.c_str(), NULL);
        if (r != 0) {
            Logger::Get()->Write(LogType::Error, Severity::Critical, uv_err_name(r));
        }

        lwt_t = (time_t) req.statbuf.st_mtim.tv_sec;

        lwt = ctime(&lwt_t);

        lwt[strlen(lwt) - 1] = '\0';


        fi->file_size = content.size();
        fi->last_write_time = lwt;

        string md5_src = file;
        md5_src += lwt;
        fi->etag = md5(md5_src);

        std::size_t last_slash_pos = file.find_last_of("/");
        std::size_t last_dot_pos = file.find_last_of(".");
        std::string extension;
        if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
            extension = file.substr(last_dot_pos + 1);
        }


        fi->content = content;
        fi->file_name = file;

        ///uncompressed headers
        fi->cached_headers += "\r\nCache-Control: public, max-age=0\r\nConnection: keep-alive\r\nServer: pigeon\r\nAccept_Range: bytes\r\n";
        GetHeaderField(HttpHeader::Content_Type, fi->cached_headers);
        fi->cached_headers += GetMimeType(extension);
        fi->cached_headers += "\r\n";

        GetHeaderField(HttpHeader::Content_Length, fi->cached_headers);
        fi->cached_headers += std::to_string(fi->file_size);
        fi->cached_headers += "\r\n";

        GetHeaderField(HttpHeader::Last_Modified, fi->cached_headers);
        fi->cached_headers += fi->last_write_time;
        fi->cached_headers += "\r\n";

        GetHeaderField(HttpHeader::ETag, fi->cached_headers);
        fi->cached_headers += fi->etag;
        fi->cached_headers += "\r\n";


        CompressItem(fi);

        ///compresses headers
        fi->compresses_cached_headers += "\r\nCache-Control: public, max-age=0\r\nConnection: keep-alive\r\nServer: pigeon\r\nAccept_Range: bytes\r\n";
        GetHeaderField(HttpHeader::Content_Encoding, fi->compresses_cached_headers);

        GetHeaderField(HttpHeader::Content_Type, fi->compresses_cached_headers);
        fi->compresses_cached_headers += GetMimeType(extension);
        fi->compresses_cached_headers += "\r\n";

        GetHeaderField(HttpHeader::Content_Length, fi->compresses_cached_headers);
        fi->compresses_cached_headers += std::to_string(fi->compressed_file_size);
        fi->compresses_cached_headers += "\r\n";

        GetHeaderField(HttpHeader::Last_Modified, fi->compresses_cached_headers);
        fi->compresses_cached_headers += fi->last_write_time;
        fi->compresses_cached_headers += "\r\n";

        GetHeaderField(HttpHeader::ETag, fi->compresses_cached_headers);
        fi->compresses_cached_headers += fi->etag;
        fi->compresses_cached_headers += "\r\n";


        cache_data.insert({file, fi});


    }
}

void FileCache::LoadFiles(string filepath, uv_fs_t *req) {

    uv_dirent_t dent;

    int r = uv_fs_scandir(uv_default_loop(), req, filepath.c_str(), 0, NULL);
    if (r < 0) {
        Logger::Get()->Write(LogType::Error, Severity::Critical, uv_err_name(r));
    }

    while (!uv_fs_scandir_next(req, &dent)) {

        string path;

        switch (dent.type) {

            case UV_DIRENT_UNKNOWN:
                break;
            case UV_DIRENT_LINK:
                break;
            case UV_DIRENT_FIFO:
                break;
            case UV_DIRENT_SOCKET:
                break;
            case UV_DIRENT_CHAR:
                break;
            case UV_DIRENT_BLOCK:
                break;
            case UV_DIRENT_FILE:

                path.clear();
                path.append(req->path);
                path.append("/");
                path.append(dent.name);

                uv_rwlock_wrlock(&cache_lock);
                CacheItem(path);
                uv_rwlock_wrunlock(&cache_lock);


                uv_fs_event_t *fs_event;
                fs_event = (uv_fs_event_t *) malloc(sizeof(uv_fs_event_t));
                fs_event->data = this;
                uv_fs_event_init(uv_default_loop(), fs_event);
                uv_fs_event_start(fs_event, refresh, path.c_str(), 0);

                break;

            case UV_DIRENT_DIR:

                path.clear();
                path.append(req->path);
                path.append("/");
                path.append(dent.name);

                uv_fs_t *fsreq = (uv_fs_t *) malloc(sizeof(uv_fs_t));
                LoadFiles(path, fsreq);

                break;

        }
    }

    uv_fs_req_cleanup(req);
}

void FileCache::Load(string path) {

    uv_fs_t *fsreq = (uv_fs_t *) malloc(sizeof(uv_fs_t));
    LoadFiles(path, fsreq);

}

void FileCache::ReloadItem(string &file) {

    map<string, FileInfo*>::iterator iter;
    for (iter = cache_data.begin(); iter != cache_data.end(); ++iter) {
        if(iter->first == file){
            uv_rwlock_wrlock(&cache_lock);
            cache_data.erase(iter);
            uv_rwlock_wrunlock(&cache_lock);
            break;
        }
    }


    uv_rwlock_wrlock(&cache_lock);
    CacheItem(file);
    uv_rwlock_wrunlock(&cache_lock);

    uv_fs_event_t *fs_event;
    fs_event = (uv_fs_event_t *) malloc(sizeof(uv_fs_event_t));
    fs_event->data = this;
    uv_fs_event_init(uv_default_loop(), fs_event);
    uv_fs_event_start(fs_event, refresh, file.c_str(), 0);

}

FileInfo* FileCache::GetItem(string &file_name) {
    return cache_data[file_name];
}

