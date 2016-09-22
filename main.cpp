#include "AppService.h"
#include "net/HttpServer.h"
#include "data/rdb/R.h"
#include "data/rdb/RdbConnection.h"

#include <iostream>

using namespace pigeon;
using namespace pigeon::net;
using namespace pigeon::data::rdb;

int hello(HttpContext* context){

    //.filter("{ \"Country\": \"US\"}")
    R r;
    r.db("aw").table("employees").filter("{ \"Country\": \"US\"}").run(context,
    [](HttpContext* context, ResultSet& rs) -> void {
        while(rs.HasRows()){
            string country = rs.GetString("Country");
            uint empId = rs.GetUint("EmployeeID");
        }
        string res = rs.ToJson();
        context->Request->CreateResponse(res.c_str(), context->Response, HttpStatus::OK);
    });

}
int main(){
    AppService a;
    a.AddRoute("/api/hello", 1, hello);
    a.Start();
	return 0;
}
