//
// Created by kamlesh on 16/10/15.
//

#ifndef PIGEON_SERVER_H
#define PIGEON_SERVER_H



using namespace std;

namespace pigeon {

    class server {

    private:

        class tcp;
        class ipc;
        tcp* _tcpImpl;
        ipc* _ipcImpl;

    public:

        server();
        ~server();
        void start();

    };


}

#endif //PIGEON_SERVER_H
