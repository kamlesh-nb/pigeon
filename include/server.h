//
// Created by kamlesh on 16/10/15.
//

#ifndef PIGEONWEB_SERVER_H
#define PIGEONWEB_SERVER_H

#include <functional>
#include "logger.h"
#include <uv.h>
#include "app.h"
#include "settings.h"

using namespace std;

namespace pigeon {

    class server {

    private:

        static function<void(uv_stream_t* socket, int status)> on_connect;
        static function<void(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)> on_read;
        static function<void(uv_work_t *req)> on_render;
        static function<void(uv_work_t *req)> on_render_complete;
        static function<void(uv_write_t *req, int status)> on_send_complete;
        static function<void(uv_handle_t *handler)> on_close;

        //setup callback lambda's
        void on_connect_cb();
        void on_read_cb();
        void on_render_cb();
        void on_render_complete_cb();
        void on_send_complete_cb();
        void on_close_cb();

        settings* m_settings;
        cache* m_cache;

        string m_log_file;

        ///setup all the uv callbacks
        void initialise();

        //initialise tcp
        void initialise_tcp();

        ///initialise all http parser
        void initialise_parser();

        ///setup thread pool environment variable
        void setup_thread_pool();

    public:

        server();
        ~server();
        void start();
        void stop();

    };



}

#endif //PIGEONWEB_SERVER_H
