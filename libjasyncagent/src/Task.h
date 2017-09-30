#ifndef TASK_H_
#define TASK_H_

#include <memory>

#include "RunnableCaller.h"

extern JavaVM* jvm;

class Task
{

    std::unique_ptr<RunnableCaller> runnable;
    jobject obj;
    JNIEnv* jniEnv;
    bool hasCompleted;

protected:

    virtual void callRunnable();
    virtual void yieldInternal() const = 0;
    virtual void runInternal() = 0;

    bool isCompleted();

    static void runInternal(Task*);

    Task(jobject);

public:

    virtual ~Task();

    // -1 means Task is completed
    virtual int run();
    virtual void finish();
    void yield() const;

    long id() {
        return (long)this;
    }

    static Task* create(JNIEnv*, jobject);
    static int getNumOfTasks();
    static bool setup();

};

#endif /* TASK_H_ */
