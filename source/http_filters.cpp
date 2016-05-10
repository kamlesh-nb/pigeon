#include "http_filters.h"

using namespace pigeon;


std::mutex http_filters::_mtx;
std::shared_ptr<http_filters> http_filters::temp = nullptr;

http_filters::http_filters() {
}

http_filters::~http_filters() {
}

http_filters::http_filters(const http_filters& filters) {
    temp = filters.temp;
}

http_filters& http_filters::operator = (const http_filters& filters) {
    if (this != &filters) {
       temp = filters.temp;
    }
    return *this;
}

void http_filters::add(const string& filter_name, CreateFilter func)
{
	registry[filter_name] = func;
}

std::shared_ptr<http_filter_base> http_filters::create(const std::string &filter_name) const
{
	std::shared_ptr<http_filter_base> _filter;
	filters::const_iterator regEntry = registry.find(filter_name);
	if (regEntry != registry.end())
	{
		_filter = regEntry->second();
	}
	return _filter;
}

std::shared_ptr<http_filters> &http_filters::instance() {
    static std::shared_ptr<http_filters> tmp = temp;

    if (!tmp) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!tmp) {
            temp.reset(new http_filters);
            tmp = temp;
        }
    }

    return tmp;
}