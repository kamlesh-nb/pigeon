#ifndef PIGEON_REQUEST_PROCESSOR_H
#define PIGEON_REQUEST_PROCESSOR_H


#include <http_context.h>


namespace pigeon {

	class request_processor
	{
	private:

		bool execute_request_filters(http_context*);
		void parse_multipart(http_context*);
        void parse_cookies(http_context*);
        void parse_url(http_context*);
		bool execute_response_filters(http_context*);
		void handle_cors(http_context *);

	public:
		request_processor();
		~request_processor();

		void process(http_context*);

	};

}



#endif //PIGEON_REQUEST_PROCESSOR_H
