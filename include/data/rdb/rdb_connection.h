//
// Created by kamlesh on 9/17/16.
//

#ifndef PIGEON_RDB_CONNECTION_H
#define PIGEON_RDB_CONNECTION_H
#include "data/rdb/R.h"
#include "data/db_connection.h"



namespace pigeon {
    namespace data {
        namespace rdb {
            class rdb_connection : public db_connection {
            private:
                void send_connect();
            public:
                void handshake(uv_connect_t* conn_req, int status);
                void on_handshake(uv_write_t* req, int status);
                virtual bool get_status() override;
                rdb_connection(uv_loop_t* loop);
                virtual ~rdb_connection() override;
                void send_query(std::string string, R* r);

                virtual int connect(std::string string, int i) override;
                virtual int connect(std::string string, int i, std::string string1) override;
                virtual void on_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) override;
                virtual void on_write(uv_write_t *req, int status) override;
            };
        }
    }
}




#endif //PIGEON_RDB_CONNECTION_H
