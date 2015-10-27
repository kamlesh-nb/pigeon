//
// Created by kamlesh on 16/10/15.
//

#ifndef PIGEON_SERVER_H
#define PIGEON_SERVER_H


using namespace std;

namespace pigeon {

    class server {

    private:

        
         
        void _init();
        void _tcp();
		void _bind();
		void _listen();
        void _parser();
		void _pool_size();

    public:

        server();
        ~server();
        void start();
        void stop();

    };



}

#endif //PIGEON_SERVER_H
