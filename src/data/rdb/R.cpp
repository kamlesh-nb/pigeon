//
// Created by kamlesh on 9/12/16.
//

#include "data/rdb/R.h"
#include "data/rdb/rdb_connection.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>


#define LOG_RUV_ERR(msg, code) do {                                                      \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);

using namespace rapidjson;
using namespace pigeon::data::rdb;

R::R(http_context*){

}

R::~R() {

}

result_set* R::parse_result(char *) {
    return nullptr;
}


void R::on_fetch_result(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
    if(nread >= 0) {

            char* c = buf->base;
            while(true){
                if(*c == '{'){
                    break;
                }
                else { ++c; }
            }
            result_set* res = parse_result(c);
            requestCb(m_http_context, res);
    }
    else {
        uv_close((uv_handle_t*)handle,
                 [](uv_handle_t* handle){

                 });
    }
    free(buf->base);
}


void R::on_query_sent(uv_write_t *req, int status){
    if(status) LOG_RUV_ERR("rdb write error", status);
    uv_read_start(req->handle,
                  [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                      *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                  },
                  [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                      R* ptr = static_cast<R*>(handle->data);
                      ptr->on_fetch_result(handle, nread, buf);
                  });

}

void R::run(db_connection* conn, request_cb func_cb) {
    msg.append("[1,");
    msg.append(child.top());
    msg.append("{}]]");
    requestCb = func_cb;
    rdb_connection* c = static_cast<rdb_connection*>(conn);
    c->send_query(msg, this);
}

R* R::table(string tablename){
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
R* R::filter(string filter_criteria){
    string cmd;
    cmd +=  "[39,[" ;
    cmd += child.top();
    cmd += filter_criteria;
    cmd +=  "]]";
    child.pop();
    child.push(cmd);

    return this;
}
R* R::insert(string data){
    string cmd;
    cmd +=   "[56,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::get(string data){
    string cmd;
    cmd +=   "[16,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::getAll(string data){
    string cmd;
    cmd +=   "[78,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::eqJoin(string data){

    string cmd;
    cmd +=   "[50,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::outerJoin(string data){
    string cmd;
    cmd +=   "[49,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::innerJoin(string data){
    string cmd;
    cmd +=   "[48,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::zip(string data){
    string cmd;
    cmd +=   "[72,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::group(string data){
    string cmd;
    cmd +=   "[144,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::orderBy(string data){
    string cmd;
    cmd +=   "[41,[";
    cmd += child.top();
    cmd += data;
    cmd += "]]";
    child.pop();
    child.push(cmd);
    return this;
}
R* R::db(string name){
    dbname += "[\"";
    dbname += name;
    dbname += "\"]]" ;
    return this;
}


