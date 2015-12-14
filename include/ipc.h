//
// Created by kamlesh on 13/12/15.
//

#ifndef PIGEON_IPC_H
#define PIGEON_IPC_H
#include <uv.h>


namespace pigeon {


    class ipc {

    private:

        class ipc_impl;
        ipc_impl* _Impl;

    public:

        ipc();
        ~ipc();
        void start();
    };

}
#endif //PIGEON_IPC_H
