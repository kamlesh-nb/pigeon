//
// Created by kamlesh on 23/10/15.
//


#include "app.h"
#include "server.h"


using namespace std;

namespace pigeon {


    auto app::run() -> void {
        settings* st = new settings;
        server s;
        s.start();

    }




}

