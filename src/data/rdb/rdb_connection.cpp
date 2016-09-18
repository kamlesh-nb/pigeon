//
// Created by kamlesh on 9/17/16.
//

#include <cstring>
#include <assert.h>
#include <iostream>
#include <sstream>


#include "data/rdb/rdb_connection.h"
#include "data/rdb/protocol_defs.h"


using namespace pigeon::data::rdb;

#define LOG_DBUV_ERR(msg, code) do {                                                      \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);

const uint32_t version_magic =
        static_cast<uint32_t>(Protocol::VersionDummy::Version::V0_4);
const uint32_t json_magic =
        static_cast<uint32_t>(Protocol::VersionDummy::Protocol::JSON);

bool rdb_connection::get_status() {
    return status;
}

rdb_connection::rdb_connection(uv_loop_t *loop) {
    client_loop = loop;
}

rdb_connection::~rdb_connection() {

}

void rdb_connection::send_query(std::string query, R* r) {
    ++token;

    char header[12];
    memcpy(header, &token, 8);
    uint32_t size = query.size();
    memcpy(header + 8, &size, 4);
    query.insert(0, header, 12);

    uv_write_t* request = (uv_write_t*)malloc(sizeof(uv_write_t));
    uv_buf_t buffer = uv_buf_init((char*)malloc(query.size()), query.size());

    memcpy(buffer.base, (char*)query.c_str(), query.size());
    buffer.len = query.size();
    request->data = r;

    int res = uv_write(request, conn_req.handle, &buffer, 1, [](uv_write_t* req, int status){
        R* ptr = static_cast<R*>(req->data);
        ptr->on_query_sent(req, status);
    });

    if(res) LOG_DBUV_ERR("rdb write error", res);

}

int rdb_connection::connect(std::string address, int port) {
    int r;
    client.data = this;
    conn_req.data = this;

    uv_tcp_init(client_loop, &client);
    uv_tcp_keepalive(&client, 1, 60);
    uv_ip4_addr(address.c_str(), port, &addr);
    r = uv_tcp_connect(&conn_req, &client, (const sockaddr*)&addr, [](uv_connect_t* conn_req, int status){
        rdb_connection* ptr = static_cast<rdb_connection*>(conn_req->data);
        ptr->handshake(conn_req, status);
    });
    return r;
}

int rdb_connection::connect(std::string string, int i, std::string string1) {
    return 0;
}

void rdb_connection::on_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
    if(nread >= 0) {
       if(nread == 8){
           status = true;
       }
    }
    else {
        uv_close((uv_handle_t*)handle,
                 [](uv_handle_t* handle){

                 });
    }
    free(buf->base);
}

void rdb_connection::on_write(uv_write_t *req, int status) {
    if(status) LOG_DBUV_ERR("rdb write error", status);
    uv_read_start(req->handle,
                  [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                        *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                  },
                  [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                        rdb_connection* ptr = static_cast<rdb_connection*>(handle->data);
                        ptr->on_read(handle, nread, buf);
                  });


}

void rdb_connection::send_connect() {
    uv_stream_t* stream = conn_req.handle;
    stream->data = this;

    std::string auth_key("");
    const size_t size = 0;

    char buf[12 + size];

    memcpy(buf, &version_magic, 4);
    uint32_t n = 0;
    memcpy(buf + 4, &n, 4);
    memcpy(buf + 8, auth_key.data(), size);
    memcpy(buf + 8 + size, &json_magic, 4);

    uv_write_t* request = (uv_write_t*)malloc(sizeof(uv_write_t));
    request->data = this;
    uv_buf_t buffer = uv_buf_init((char*)malloc(12), 12);
    memcpy(buffer.base, buf, 12);
    buffer.len = 12;

    int r = uv_write(request, stream, &buffer, 1, [](uv_write_t* req, int status){
        rdb_connection* _ptr = static_cast<rdb_connection*>(req->data);
        _ptr->on_handshake(req, status);
    });

}

void rdb_connection::handshake(uv_connect_t *conn_req, int status) {
    if(status) LOG_DBUV_ERR("rdb write error", status);
    send_connect();
}

void rdb_connection::on_handshake(uv_write_t *req, int status) {

    if(status) LOG_DBUV_ERR("rdb write error", status);
    uv_read_start(req->handle,
                  [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                        *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                  },
                  [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                        rdb_connection* ptr = static_cast<rdb_connection*>(handle->data);
                        ptr->on_read(handle, nread, buf);
                  });

}



