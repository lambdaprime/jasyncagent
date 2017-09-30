#include "Queue.h"
#include "RunnableCaller.h"
#include "BoostTask.h"

#include <iostream>
#include <thread>

using namespace std;

BoostTask::BoostTask(std::unique_ptr<RunnableCaller> runnable) :
    coroutine(nullptr),
    sink(nullptr),
    runnable(move(runnable))
{

}

extern JavaVM *jvm;

int BoostTask::get() {
    if (!runnable) {
        cout << "runnable is empty, skipping..." << endl;
        return -1;
    }
    //runnable->call();
    if (!coroutine) {
        cout << "initing coro" << std::this_thread::get_id() << endl;
        coroutine = unique_ptr<coroutine_t::pull_type>(new coroutine_t::pull_type([this](coroutine_t::push_type& sink){
            cout << "-task started" << std::this_thread::get_id() << endl;
            JNIEnv* looperEnv;
            if (jvm->AttachCurrentThread((void**)&looperEnv, NULL) != JNI_OK) {
                cerr << "cannot attach looper thread to JVM" << endl;
            }
            //this->sink = &sink;
            // Runnable.call
            runnable->call(looperEnv);
/*
            cout << "-task suspended" << endl;
            queue.push(this);
            sink(123);
            cout << "-task was resumed" << endl;
            //sink(100);
*/
            cout << "-task finishing" << endl;
        }));
    } else {
        //coroutine->operator ()();
    }
    //if (!coroutine) return -1;
    //return coroutine->get();
    return -1;
}

#include <boost/asio/write.hpp>

using namespace boost::asio;

void BoostTask::async_write() {
    async_write();
}
