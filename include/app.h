//
// Created by kamlesh on 9/17/16.
//

#ifndef PIGEON_APP_H
#define PIGEON_APP_H

#include <functional>
#include <memory>
#include <mutex>
#include <uv.h>
#include "http_context.h"

using namespace std;

namespace pigeon {

    typedef std::function<int(pigeon::net::http_context*)> request_process_step_cb;

    class app {
    private:

    public:
        /*
         *  route parameter should be route of the api excluding the api_route part fromthe settings
         *  Following are the values that should be followed for method parameter
         *  0 = DELETE
         *  1 = GET
         *  3 = POST
         *  4 = PUT
         *  func parameter should eb passed with free-function with signature as under
         *      int <func_name>(pigeon::net::http_context*);
         */
        void add_route(std::string route, int method, request_process_step_cb func);
        void add_filter(std::string, request_process_step_cb);
        int start();
        app();
        ~app();
    };
}



#endif //PIGEON_APP_H
