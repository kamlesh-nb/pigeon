//
// Created by kamlesh on 5/10/15.
//

#ifndef PIGEONWEB_APP_SERVICE_H
#define PIGEONWEB_APP_SERVICE_H

#include <mutex>
#include <string>
#include <memory>

using namespace std;
namespace pigeon {

    class app_service {

    private:

        void load_config();
        void load_cache();

        string config_path;

        static std::mutex _mtx;
        static std::shared_ptr<app_service> instance;

    public:

        void initialise();

        void add_api_handler();

        void get_app_setting(string&);

        void run();

        static std::shared_ptr<app_service> &get();

    };

}
#endif //PIGEONWEB_APP_SERVICE_H
