//
// Created by kamlesh on 9/17/16.
//

#ifndef PIGEON_RDB_CONNECTION_H
#define PIGEON_RDB_CONNECTION_H

#include <functional>
#include "data/IDbConnection.h"
#include "HttpContext.h"
#include "data/rdb/ResultSet.h"
using namespace pigeon::net;

namespace pigeon {
    namespace data {
        namespace rdb {
            class RdbConnection : public IDbConnection {
            private:
                HttpContext* mHttpContext;
                std::function<void(HttpContext*, ResultSet&)> mOnQueryComplete;
                void SendConnectPacket();
            public:
                void Handshake(uv_connect_t *conn_req, int status);
                void OnHandshake(uv_write_t *req, int status);
                virtual bool GetStatus() override;
                RdbConnection(uv_loop_t* loop);
                virtual ~RdbConnection() override;
                void SendQuery(std::string query, HttpContext* context, std::function<void(HttpContext*, ResultSet&)> pOnQueryComplete);
                ResultSet ParseResult(char*);
                void OnQuerySent(uv_write_t *req, int status);
                void OnFetchResult(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
                virtual int Connect(std::string string, int i) override;
                virtual int Connect(std::string string, int i, std::string string1) override;
                virtual void OnRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) override;
                virtual void OnWrite(uv_write_t *req, int status) override;
            };
        }
    }
}




#endif //PIGEON_RDB_CONNECTION_H
