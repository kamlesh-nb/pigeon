//
// Created by kamlesh on 16/10/15.
//

#ifndef PIGEON_SERVER_H
#define PIGEON_SERVER_H

#include <uv.h>
#include <http_filter_base.h>
#include <http_handler_base.h>
#include <resource_handler.h>
#include <unordered_map>


using namespace std;

namespace pigeon {



    class server {

    private:
        class server_impl;
        server_impl* _Impl;

    public:

        server();
        ~server();
        void start();
        void add_handler(string&, http_handler_base*);
        void add_filter(string&, http_filter_base*);

    };



}

#endif //PIGEON_SERVER_H
