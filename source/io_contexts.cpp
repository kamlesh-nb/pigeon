//
// Created by kamlesh on 23/11/15.
//

#include "io_contexts.h"
#include <boost/bind.hpp>

using  namespace pigeon::tcp;
using namespace std;


io_contexts::io_contexts(std::size_t pool_size)
        : next_io_context_(0)
{
    if (pool_size == 0)
        throw std::runtime_error("io_contexts size is 0");

    // Give all the io_contexts work to do so that their run() functions will not
    // exit until they are explicitly stopped.
    for (std::size_t i = 0; i < pool_size; ++i)
    {
        io_context_ptr io_context(new asio::io_context);
        work_ptr work(new asio::io_context::work(*io_context));
        io_contexts_.push_back(io_context);
        work_.push_back(work);
    }
}

void io_contexts::run()
{
    std::vector<shared_ptr<asio::thread> >  workers;
    for (int i = 0; i < io_contexts_.size(); i++) {

        shared_ptr<asio::thread> thread(new asio::thread(
                boost::bind(&asio::io_context::run, io_contexts_[i])));

        workers.push_back(thread);
    }

    for (std::size_t i = 0; i < workers.size(); ++i)
        workers[i]->join();

}

void io_contexts::stop()
{
    // Explicitly stop all io_contexts.
    for (std::size_t i = 0; i < io_contexts_.size(); ++i)
        io_contexts_[i]->stop();
}

asio::io_context& io_contexts::get_io_context()
{
    // Use a round-robin scheme to choose the next io_context to use.
    asio::io_context& io_context = *io_contexts_[next_io_context_];
    ++next_io_context_;
    if (next_io_context_ == io_contexts_.size())
        next_io_context_ = 0;
    return io_context;
}
