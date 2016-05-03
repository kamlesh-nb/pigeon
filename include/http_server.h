#ifndef PIGEON_HTTP_SERVER_H
#define PIGEON_HTTP_SERVER_H

#include <request_processor.h>

namespace pigeon {

	class http_server
	{
	
	private:

		class tcp;
		class pipe;

		tcp* tcpImpl;
		pipe* pipeImpl;

		request_processor* RequestProcessor;

		void _init();
		void _parser();
	
	public:

		http_server();
		~http_server();

		void run();
	

	};

	
}


#endif //PIGEON_HTTP_SERVER_H
