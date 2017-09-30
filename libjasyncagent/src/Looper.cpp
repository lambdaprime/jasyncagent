#include <utility>

#include "Looper.h"
#include "AsyncAgentJni.h"
#include "RunnableCaller.h"
#include "Logger.h"
#include "AsyncAgent.h"

using namespace std;

static Logger logger("Looper");

Looper::Looper() :
    isStarted(false),
    currentTask(nullptr)
{
}

void Looper::consume(Task* task) {
    traceIn;
    currentTask = task;
    int v = task->run();
    logger.dbg() << "Task returned " << std::dec << v << endl;
    if (v == -1)
        delete task;
    currentTask = nullptr;
    logger.dbg() << "Queue size is " << std::dec << queue.size() << endl;
}

void Looper::start(JavaVM *jvm) {
    isStarted = true;
    th = thread([this, jvm]{
        logger.debug("Thread started.");
        while (true) {
            queue.consume([this](Task* t){
                consume(t);
            });
            if (!isStarted && Task::getNumOfTasks() == 0)
                break;
        }
        logger.debug("Thread stopped.");
    });
}

void Looper::stop() {
    isStarted = false;
    queue.notify();
    th.join();
}

bool Looper::isOnLooperThread() {
    return this_thread::get_id() == th.get_id();
}
