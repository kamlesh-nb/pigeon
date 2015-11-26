#include "http_filters.h"

using namespace pigeon;


std::mutex http_filters::_mtx;
std::shared_ptr<http_filters> http_filters::temp = nullptr;

http_filters::http_filters()
{
}

http_filters::~http_filters()
{
}

http_filters::http_filters(const http_filters &filter){

}


void http_filters::add(string filter_name, http_filter_base* filter){
	filters.emplace(std::pair<string, http_filter_base*>(filter_name, filter));
}

http_filter_base* http_filters::get(string& filter_name){
	return filters[filter_name];
}

 

std::shared_ptr<http_filters>&http_filters::instance()
{
	static std::shared_ptr<http_filters> tmp = temp;

	if (!tmp)
	{
		std::lock_guard<std::mutex> lock(_mtx);
		if (!tmp)
		{
			temp.reset(new http_filters);
			tmp = temp;
		}
	}

	return tmp;
}