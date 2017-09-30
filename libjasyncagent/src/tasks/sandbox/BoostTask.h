#ifndef BOOSTTASK_H_
#define BOOSTTASK_H_

#include <boost/coroutine2/coroutine.hpp>
#include <memory>


typedef boost::coroutines2::coroutine<int> coroutine_t;

class RunnableCaller;

class BoostTask
{

    std::unique_ptr<coroutine_t::pull_type> coroutine;
    coroutine_t::push_type* sink;

    std::unique_ptr<RunnableCaller> runnable;

    void async_write();

public:

    BoostTask(std::unique_ptr<RunnableCaller>);
    //Task(const Task&);
    int get();
    //~Task();

};

#endif /* BOOSTTASK_H_ */
