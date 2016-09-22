//
// Created by kamlesh on 9/17/16.
//

#ifndef PIGEON_DB_CONNECTION_H
#define PIGEON_DB_CONNECTION_H

#include <memory>
#include <string>
#include <uv.h>


namespace pigeon {

    namespace data {

        class IDbConnection {
        protected:
            std::string m_address;
            int m_port;
            struct sockaddr_in addr;
            int64_t token;
            bool status = false;
            uv_loop_t* client_loop;
            uv_tcp_t client;
            uv_connect_t conn_req;
            std::string auth_key;
            uv_buf_t cbuff;
        public:
            virtual bool GetStatus() = 0;
            virtual ~IDbConnection(){};
            virtual int Connect(std::string, int) = 0;
            virtual int Connect(std::string, int, std::string) = 0;
            virtual void OnRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) = 0;
            virtual void OnWrite(uv_write_t *req, int status) = 0;

        };

    }

}

#endif //PIGEON_DB_CONNECTION_H
