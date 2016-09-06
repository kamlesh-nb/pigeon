#include "app_context.h"
#include "http_util.h"

#include <string>
#include <fstream>
#include <assert.h>

using namespace pigeon;

class app_context::logger {
private:
    uv_rwlock_t rw_lock;
    std::ofstream lf;
public:
    logger() {
        int r = uv_rwlock_init(&rw_lock);
        assert(!r);

        if (!lf.is_open()) {
            lf.open(app_context::app_log_location, ios::app);
        }
    }
    ~logger() {

    }
    void write(LogType type, Severity severity, string message) {

        uv_rwlock_rdlock(&rw_lock);
        lf << get_log_type(type) << ": " << get_severity(severity) << ": " << now() << " : " << message << endl;
        lf.flush();
        uv_rwlock_rdunlock(&rw_lock);

    }
    void write(LogType type, string message) {
        uv_rwlock_rdlock(&rw_lock);
        lf << get_log_type(type) << ": " << now() << " : " << message << endl;
        lf.flush();
        uv_rwlock_rdunlock(&rw_lock);
    }
    void close() {
        if (lf.is_open()) {
            lf.close();
        }
    }

};