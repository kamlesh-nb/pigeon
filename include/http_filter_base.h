//
// Created by kamlesh on 28/10/15.
//

#ifndef PIGEON_HTTP_FILTER_BASE_H
#define PIGEON_HTTP_FILTER_BASE_H

#include "http_context.h"

namespace pigeon {

    class http_filter_base {
    public:
        virtual ~http_filter_base(){};
        virtual void init() = 0;
        virtual bool execute(http_context *) = 0;
        virtual void clean() = 0;
    };

    template<typename T>
    inline std::shared_ptr<http_filter_base> createfiltler() {
        return std::make_shared<T>();
    }

}

#endif //PIGEON_HTTP_FILTER_BASE_H
