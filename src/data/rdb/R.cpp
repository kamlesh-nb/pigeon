//
// Created by kamlesh on 9/22/16.
//

#include <net/HttpServer.h>
#include "data/rdb/R.h"
#include "data/rdb/RdbConnection.h"

using namespace pigeon::data::rdb;
using namespace pigeon::net;

R::R(){}
R::~R() {

}
R R::db(string name){
    dbname += "[\"";
    dbname += name;
    dbname += "\"]]" ;
    return *this;
}
R R::table(string tablename){
    string cmd;
    if (dbname.size()){
        cmd += "[15,[[14,";
        cmd += dbname;
        cmd += ",\"";
        cmd += tablename;
        cmd += "\"]";
    }
    else
    {
        cmd +=  "[[14,";
        cmd += ",\"";
        cmd += tablename;
        cmd += "\"]]";
    }
    child.push(cmd);
    return *this;
}
R R::filter(string filter_criteria){
    string cmd;
    cmd +=  "[39,[" ;
    cmd += child.top();
    cmd += "],";
    cmd += filter_criteria;
    cmd +=  "]";
    child.pop();
    child.push(cmd);

    return *this;
}
R R::insert(string data){
    string cmd;
    cmd +=   "[56,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::get(string data){
    string cmd;
    cmd +=   "[16,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::getAll(string data){
    string cmd;
    cmd +=   "[78,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::eqJoin(string data){

    string cmd;
    cmd +=   "[50,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::outerJoin(string data){
    string cmd;
    cmd +=   "[49,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::innerJoin(string data){
    string cmd;
    cmd +=   "[48,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::zip(string data){
    string cmd;
    cmd +=   "[72,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::group(string data){
    string cmd;
    cmd +=   "[144,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}
R R::orderBy(string data){
    string cmd;
    cmd +=   "[41,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return *this;
}

void R::run(HttpContext* context, std::function<void(HttpContext*, ResultSet&)> pOnQueryComplete) {
    msg.append("[1,");
    msg.append(child.top());
    msg.append("],{}]");
    client_t* client = static_cast<client_t*>(context->data);
    RdbConnection* ptrRdbConnection = static_cast<RdbConnection*>(client->dbConnection);
    ptrRdbConnection->SendQuery(msg, context, pOnQueryComplete);
}
