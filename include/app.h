//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_APP_ENV_H
#define PIGEON_APP_ENV_H

#include <settings.h>
#include <http_filter_base.h>

using namespace std;

namespace pigeon {



    class app {

    public:

        auto run() -> void;
		auto add_http_filter(http_filter_base*) -> void;


    };

}



#endif //PIGEON_APP_ENV_H
