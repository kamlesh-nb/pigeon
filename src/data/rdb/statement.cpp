//
// Created by kamlesh on 9/12/16.
//

#include "data/rdb/statement.h"


using namespace pigeon::data::rdb;

statement::statement(rdb_connection* conn) : mConnection(conn){

}

statement::~statement() {

}


void statement::run() {
    msg.append("[1,");
    msg.append(child.top());
    msg.append("{}]]");
    mConnection->send_query(msg);

}

statement* statement::table(string tablename){
    string cmd;
    if (dbname.size()){
        cmd += "[15,[[14,";
        cmd += dbname;
        cmd += ",\"";
        cmd += tablename;
        cmd += "\"],";
    }
    else
    {
        cmd +=  "[[14,";
        cmd += ",\"";
        cmd += tablename;
        cmd += "\"]],";
    }
    child.push(cmd);
    return this;
}
statement* statement::filter(string filter_criteria){
    string cmd;
    cmd +=  "[39,[" ;
    cmd += child.top();
    cmd += filter_criteria;
    cmd +=  "]]";
    child.pop();
    child.push(cmd);

    return this;
}
statement* statement::insert(string data){
    string cmd;
    cmd +=   "[56,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::get(string data){
    string cmd;
    cmd +=   "[16,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::getAll(string data){
    string cmd;
    cmd +=   "[78,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::eqJoin(string data){

    string cmd;
    cmd +=   "[50,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::outerJoin(string data){
    string cmd;
    cmd +=   "[49,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::innerJoin(string data){
    string cmd;
    cmd +=   "[48,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::zip(string data){
    string cmd;
    cmd +=   "[72,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::group(string data){
    string cmd;
    cmd +=   "[144,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::orderBy(string data){
    string cmd;
    cmd +=   "[41,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
statement* statement::db(string name){
    dbname += "[\"";
    dbname += name;
    dbname += "\"]]" ;
    return this;
}


