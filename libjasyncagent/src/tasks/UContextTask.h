#ifndef UCONTEXTTASK_H_
#define UCONTEXTTASK_H_

#include <memory>
#include <ucontext.h>
#include <pthread.h>

#include "Task.h"
#include "Message.h"

class UContextTask: public Task
{

    std::unique_ptr<ucontext_t> childContext;
    std::unique_ptr<ucontext_t> mainContext;
    void* stack;
    ThreadStorageManager manager;

    friend void runInternal_(UContextTask* task);

    void beforeSwap();
    void afterSwap();

    UContextTask(jobject, void*);

protected:

    void yieldInternal() const;
    void runInternal();

public:

    virtual ~UContextTask();

    static std::unique_ptr<UContextTask> create(jobject);
    static bool initialize();

};

#endif /* UCONTEXTTASK_H_ */
