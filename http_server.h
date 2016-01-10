//
// Created by kamlesh on 30/10/15.
//

#ifndef PIGEON_HTTP_SERVER_H
#define PIGEON_HTTP_SERVER_H


namespace pigeon {

    class http_server {

    private:

        void init_parser();
        void init_server();

    public:

        void start();

    };

}



#endif //PIGEON_HTTP_SERVER_H
