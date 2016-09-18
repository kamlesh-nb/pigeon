#include <net/http_server.h>
#include "data/rdb/statement.h"
#include "data/rdb/rdb_connection.h"
#include "app.h"
using namespace pigeon;
using namespace pigeon::net;
using namespace pigeon::data::rdb;

int hello(http_context* context){

    client_t* client = static_cast<client_t*>(context->data);
    rdb_connection* rdbConnection = static_cast<rdb_connection*>(client->dbConnection);
    statement* stmt = new statement(rdbConnection);

    stmt->db("aw")->table("employees")->run();

    std::string msg;
    msg += "{ \"message\": \"Hello World!!\" }";
    context->request->create_response(msg, context->response, HttpStatus::OK, false);
}
int main(){
    app a;
    a.add_route("/api/hello", 1, hello);
    a.start();
	return 0;
}
