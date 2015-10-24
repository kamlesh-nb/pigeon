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
#include <md5.h>
#include "http_util.h"
#include <logger.h>
#include "app.h"
#include "file_info.h"
#include "server.h"


using namespace std;

namespace pigeon {


    auto app::run() -> void {

        server s(this);
        s.start();

    }

    auto app::create() -> void {
        m_app_settings = new settings;
        m_resource_cache = new cache;
        m_app_settings->load_setting();
        m_resource_cache->load(m_app_settings->get_resource_location());
    }

    auto app::get_resource_cache() -> cache * {
        return m_resource_cache;
    }

    auto app::get_settings() -> settings * {
        return m_app_settings;
    }

}

