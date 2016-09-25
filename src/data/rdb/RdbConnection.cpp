//
// Created by kamlesh on 9/17/16.
//

#include <cstring>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <net/HttpServer.h>

#include "net/HttpServer.h"
#include "data/rdb/RdbConnection.h"
#include "data/rdb/protocol_defs.h"

using namespace pigeon::data::rdb;
using namespace pigeon::net;

#define LOG_DBUV_ERR(msg, code) do {                                                      \
	fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
	assert(0);                                                                          \
} while (0);

const uint32_t version_magic =
        static_cast<uint32_t>(Protocol::VersionDummy::Version::V0_4);
const uint32_t json_magic =
        static_cast<uint32_t>(Protocol::VersionDummy::Protocol::JSON);




bool RdbConnection::GetStatus() {
    return status;
}

RdbConnection::RdbConnection(uv_loop_t *loop) {
    client_loop = loop;
}

RdbConnection::~RdbConnection() {
}


void RdbConnection::OnQuerySent(uv_write_t *req, int status){
    if(status) LOG_DBUV_ERR("rdb write error", status);
    uv_read_start(req->handle,
                  [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                      *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                  },
                  [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                      RdbConnection* ptr = static_cast<RdbConnection*>(handle->data);
                      ptr->OnFetchResult(handle, nread, buf);
                  });

}

void RdbConnection::OnFetchResult(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf){
    if(nread >= 0) {
        ResultSet resultSet;
        resultSet.ParseResult(buf->base);
        mOnQueryComplete(mHttpContext, resultSet);
        client_t* client = static_cast<client_t*>(mHttpContext.data);
      //  client->requestHandler->ExecuteResponseFilters(&mHttpContext);

        uv_buf_t resbuf;
        resbuf.base = (char*) client->context->Response->buffer->ToCStr();
        resbuf.len = (unsigned long) client->context->Response->buffer->GetLength();
        std::cout << client->stream.type << std::endl;
        client->write_req.data = client;
        if (uv_is_writable((uv_stream_t*)&client->stream)) {
            int r = uv_write(&client->write_req,
                             (uv_stream_t *) &client->stream,
                             &resbuf,
                             1,
                             [](uv_write_t *req, int status) {
                                 if (!uv_is_closing((uv_handle_t *) req->handle)) {
                                     uv_close((uv_handle_t *) req->handle,
                                              [](uv_handle_t *handle) {
                                                  client_t *client = (client_t *) handle->data;
                                                  client->context->Response->buffer->Clear();
                                                  delete client->context->Response->buffer;
                                                  delete client->context;
                                                  free(client);
                                              });
                                 }

                             });
        } else {
            uv_close((uv_handle_t *)&client->stream,
                     [](uv_handle_t *handle) {
                         client_t *client = (client_t *) handle->data;
                         client->context->Response->buffer->Clear();
                         delete client->context->Response->buffer;
                         delete client->context;
                         free(client);
                     });
        }
//        client->requestHandler->FinishProcess(&mHttpContext);
    }
    else {
        uv_close((uv_handle_t*)handle, [](uv_handle_t* handle){});
    }
    free(buf->base);
}

void RdbConnection::SendQuery(std::string query, HttpContext& context, std::function<void(HttpContext&, ResultSet&)> pOnQueryComplete) {
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
    request->data = this;
    mOnQueryComplete = pOnQueryComplete;
    mHttpContext = context;
    int res = uv_write(request, conn_req.handle, &buffer, 1, [](uv_write_t* req, int status){
        RdbConnection* ptr = static_cast<RdbConnection*>(req->data);
        ptr->OnQuerySent(req, status);
    });

    if(res) LOG_DBUV_ERR("rdb write error", res);

}

int RdbConnection::Connect(std::string address, int port) {
    int r;
    client.data = this;
    conn_req.data = this;

    uv_tcp_init(client_loop, &client);
    uv_tcp_keepalive(&client, 1, 60);
    uv_ip4_addr(address.c_str(), port, &addr);
    r = uv_tcp_connect(&conn_req, &client, (const sockaddr*)&addr, [](uv_connect_t* conn_req, int status){
        RdbConnection* ptr = static_cast<RdbConnection*>(conn_req->data);
        ptr->Handshake(conn_req, status);
    });
    return r;
}

int RdbConnection::Connect(std::string string, int i, std::string string1) {
    return 0;
}

void RdbConnection::OnRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
    if(nread >= 0) {
       if(nread == 8){
           status = true;
       }
    }
    else {
        uv_close((uv_handle_t*)handle, [](uv_handle_t* handle){});
    }
    free(buf->base);
}

void RdbConnection::OnWrite(uv_write_t *req, int status) {
    if(status) LOG_DBUV_ERR("rdb write error", status);
    uv_read_start(req->handle,
                  [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                        *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                  },
                  [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                        RdbConnection* ptr = static_cast<RdbConnection*>(handle->data);
                      ptr->OnRead(handle, nread, buf);
                  });


}

void RdbConnection::SendConnectPacket() {
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
        RdbConnection* _ptr = static_cast<RdbConnection*>(req->data);
        _ptr->OnHandshake(req, status);
    });

}

void RdbConnection::Handshake(uv_connect_t *conn_req, int status) {
    if(status) LOG_DBUV_ERR("rdb write error", status);
    SendConnectPacket();
}

void RdbConnection::OnHandshake(uv_write_t *req, int status) {

    if(status) LOG_DBUV_ERR("rdb write error", status);
    uv_read_start(req->handle,
                  [](uv_handle_t* /*handle*/, size_t suggested_size, uv_buf_t* buf) {
                        *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
                  },
                  [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf){
                        RdbConnection* ptr = static_cast<RdbConnection*>(handle->data);
                      ptr->OnRead(handle, nread, buf);
                  });

}




