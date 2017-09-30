#ifndef REACTOR_H_
#define REACTOR_H_

#include <thread>

#include "Task.h"

using std::thread;

class Reactor
{

    volatile bool isStarted;
    thread th;
    int efd;

public:

    enum Action {
        WRITE,
        READ
    };

    Reactor();

    bool add(int socket, Task*, Action, char const** outErrorMessage);
    bool start();
    void stop();

    virtual ~Reactor();

};

#endif /* REACTOR_H_ */
