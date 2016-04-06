#include "request_processor.h"

using namespace pigeon;



request_processor::request_processor()
{
}


request_processor::~request_processor()
{
}

void request_processor::process(http_context *context)
{
}

bool request_processor::execute_request_filters(http_context *)
{
	return false;
}

void request_processor::parse_multipart(http_context *)
{
}

bool request_processor::execute_response_filters(http_context *)
{
	return false;
}

void request_processor::handle_cors_request(http_context *)
{
}

