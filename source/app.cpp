//
// Created by kamlesh on 23/10/15.
//

#include <string>
#include <vector>
#include <uv.h>
#include <string.h>
#include <zlib.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include "http_util.h"
#include <logger.h>
#include "app.h"
#include "file_info.h"
#include "server.h"


using namespace std;

namespace pigeon {


    auto app::run() -> void {

        server s;
        s.start();

    }




}

