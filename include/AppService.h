//
// Created by kamlesh on 9/17/16.
//

#ifndef PIGEON_APP_H
#define PIGEON_APP_H

#include <functional>
#include <memory>
#include <mutex>
#include <uv.h>
#include "HttpContext.h"

using namespace std;

namespace pigeon {
    typedef std::function<int(pigeon::net::HttpContext*)> OnHandlerExecution;
    class AppService {
    private:
    public:
        /*
         *  route parameter should be route of the api excluding the api_route part fromthe Settings
         *  Following are the values that should be followed for method parameter
         *  0 = DELETE
         *  1 = GET
         *  3 = POST
         *  4 = PUT
         *  func parameter should eb passed with free-function with signature as under
         *      int <func_name>(pigeon::net::HttpContext*);
         */
        void AddRoute(std::string route, int method, OnHandlerExecution func);
        void AddFilter(std::string, OnHandlerExecution);
        int Start();
        AppService();
        ~AppService();
    };
}



#endif //PIGEON_APP_H
