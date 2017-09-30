#ifndef LOOPER_H_
#define LOOPER_H_

#include <thread>

#include "jni.h"

#include "Task.h"

using std::thread;

class Looper
{

    volatile bool isStarted;
    volatile Task* currentTask;
    thread th;

    void consume(Task* t);

public:

    Looper();
    virtual ~Looper() {};

    void start(JavaVM* jvm);
    void stop();

    Task* getCurrentTask() {
        return (Task*)currentTask;
    }

    bool isOnLooperThread();
};

#endif /* LOOPER_H_ */
