#ifndef PIGEON_HTTP_SERVER_H
#define PIGEON_HTTP_SERVER_H
 

namespace pigeon {

	class http_server
	{
	
	private:
		class http_server_impl;
		http_server_impl* _Impl;

	public:

		http_server();
		~http_server();

		void start();
	

	};

	
}


#endif //PIGEON_HTTP_SERVER_H
