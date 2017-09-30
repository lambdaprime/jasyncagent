#ifndef PTHTASK_H_
#define PTHTASK_H_

#include <memory>
#include <pth.h>

#include "Task.h"

class PthTask: public Task
{

    pth_t pth;

    friend void* runInternal_(PthTask*);

public:

    PthTask(jobject);
    //Task(const Task&);
    int run();
    //~Task();

};

#endif /* PTHTASK_H_ */
