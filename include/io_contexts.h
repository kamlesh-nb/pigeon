//
// Created by kamlesh on 23/11/15.
//

#ifndef PIGEON_IO_CONTEXTS_H
#define PIGEON_IO_CONTEXTS_H

#include <asio.hpp>
#include <vector>
#include <memory>

using namespace std;

namespace pigeon {

    namespace tcp {

        class io_contexts {
        public:
            /// Construct the io_context pool.
            explicit io_contexts(std::size_t);

            /// Run all io_context objects in the pool.
            void run();

            /// Stop all io_context objects in the pool.
            void stop();

            /// Get an io_context to use.
            asio::io_context& get_io_context();

        private:
            typedef shared_ptr<asio::io_context> io_context_ptr;
            typedef shared_ptr<asio::io_context::work> work_ptr;

            /// The pool of io_contexts.
            vector<io_context_ptr> io_contexts_;

            /// The work that keeps the io_contexts running.
            vector<work_ptr> work_;

            /// The next io_context to use for a connection.
            size_t next_io_context_;
        };

    }

}


#endif //PIGEON_IO_CONTEXTS_H
