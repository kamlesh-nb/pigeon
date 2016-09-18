#include <net/http_server.h>
#include "data/rdb/rdb_connection.h"
#include "app.h"
using namespace pigeon;
using namespace pigeon::net;
using namespace pigeon::data::rdb;

int hello(http_context* context){

    client_t* client = static_cast<client_t*>(context->data);
    rdb_connection* rdbConnection = static_cast<rdb_connection*>(client->dbConnection);
    R r(context);

    r.db("aw")->table("employees")->run(rdbConnection, [](http_context* context, result_set* rs){

    });
}
int main(){
    app a;
    a.add_route("/api/hello", 1, hello);
    a.start();
	return 0;
}
