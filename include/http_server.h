#ifndef PIGEON_HTTP_SERVER_H
#define PIGEON_HTTP_SERVER_H



namespace pigeon {

	class http_server
	{
	
	private:

		class tcp;
		class pipe;

		tcp* tcpImpl;
		pipe* pipeImpl;



		void _init();
		void _parser();
	
	public:

		http_server();
		~http_server();

		void run();
	

	};

	
}


#endif //PIGEON_HTTP_SERVER_H
