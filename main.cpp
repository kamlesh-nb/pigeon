//
// Created by kamlesh on 24/10/15.
//
#include <server.h>
#include <http_handlers.h>
#include "polltopics.h"


using namespace pigeon;

int main(){

    server s;

    http_handlers::instance()->add("/api/polltopics", new polltopics);
    s.start();
    return 0;

}
