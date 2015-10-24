//
// Created by kamlesh on 23/10/15.
//

#ifndef PIGEON_APP_ENV_H
#define PIGEON_APP_ENV_H

#include <map>
#include "cache.h"
#include "settings.h"


using namespace std;

namespace pigeon {

    class app {

    private:

        cache * m_resource_cache;
        settings * m_app_settings;

    public:

        auto create() -> void;
        auto get_resource_cache() -> cache*;
        auto get_settings() -> settings*;
        auto run() -> void;

    };

}



#endif //PIGEON_APP_ENV_H
