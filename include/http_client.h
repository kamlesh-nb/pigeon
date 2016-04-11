#ifndef PIGEON_HTTP_CLIENT_H
#define PIGEON_HTTP_CLIENT_H
#include <http_msg.h>

namespace pigeon {

	class http_client
	{

	private:

		void _init();
		void _parser();


	public:
		http_client();
		~http_client();

		int connect();
		http_response* create_request(url&);
	};

}


#endif //PIGEON_HTTP_CLIENT_H

