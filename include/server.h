//
// Created by kamlesh on 16/10/15.
//

#ifndef PIGEON_SERVER_H
#define PIGEON_SERVER_H


using namespace std;

namespace pigeon {

    class server {

    private:

        
        ///setup all the uv callbacks
        void initialise();

        //initialise tcp
        void initialise_tcp();

        ///initialise all http parser
        void initialise_parser();

        ///setup thread pool environment variable
		void set_thread_pool_env();

    public:

        server();
        ~server();
        void start();
        void stop();

    };



}

#endif //PIGEON_SERVER_H
