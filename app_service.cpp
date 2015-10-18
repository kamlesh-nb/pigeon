//
// Created by kamlesh on 5/10/15.
//

#include <sstream>
#include <iterator>
#include <unistd.h>
#include "app_service.h"
#include "server.h"
#include "config.h"
#include "logger.h"


using namespace pigeon;





void app_service::add_api_handler() {

}

void app_service::run() {

    initialise();
    server srv;
    srv.start();

}

void app_service::initialise() {
    load_config();

}

void app_service::get_app_setting(string &key) {

}

void app_service::load_cache() {

}

void app_service::load_config() {

    string config_path;
    char* cwd = get_current_dir_name();
    config_path.append(cwd);
    config_path.append("/service.json");
    config::get()->load_config(config_path);

}

std::shared_ptr<app_service> app_service::instance = nullptr;

std::mutex app_service::_mtx;

std::shared_ptr<app_service> &app_service::get() {

    static std::shared_ptr<app_service> tmp = instance;

    if (!tmp)
    {
        std::lock_guard<std::mutex> lock(_mtx);

        if (!tmp)
        {
            instance.reset(new app_service());
            tmp = instance;
        }
    }
    return tmp;

}
