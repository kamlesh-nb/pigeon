//
// Created by kamlesh on 24/10/15.
//
#include <server.h>
#include "tcp_server.h"

using namespace pigeon;

int main(){

    tcp::server s("127.0.0.1", "8080", 1);

    s.run();

    return 0;

}
