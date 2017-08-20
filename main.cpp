#include "AppService.h"
#include "net/HttpServer.h"


#include <iostream>

using namespace pigeon;
using namespace pigeon::net;

int hello(HttpContext* context){

    
    return 0;
}
int main(){
    AppService a;
    a.AddRoute("/api/hello", 1, hello);
    a.Start();
	return 0;
}
