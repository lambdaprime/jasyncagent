#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <algorithm>

#include "Reactor.h"
#include "Logger.h"
#include "AsyncAgent.h"

#define EPOLL_WAIT_TIMEOUT 1000
#define EPOLL_EVENTS_BUFFER_SIZE 5

using namespace std;

static Logger logger("Reactor");

typedef pair<Task*, int> data_t;

Reactor::Reactor() :
    isStarted(false),
    efd(0)
{

}

Reactor::~Reactor()
{

}

bool makeNonBlocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags < 0) return false;
    if ((flags & O_NONBLOCK) == O_NONBLOCK) return true;
    flags |= O_NONBLOCK;
    return fcntl(socket, F_SETFL, flags) == 0? true: false;
}

bool Reactor::add(int socket, Task* task, enum Action action, char const** errMsg) {
    traceIn;
    if (!makeNonBlocking(socket)) {
        *errMsg = "Failed to move the socket to the nonblocking mode.";
        return false;
    }
    epoll_event event;
    switch (action) {
    case READ:
        event.events = EPOLLIN;
        break;
    case WRITE:
        event.events = EPOLLOUT;
        break;
    default:
        return false;
    }
    event.data.ptr = new data_t(task, socket);
    if (epoll_ctl(efd, EPOLL_CTL_ADD, socket, &event) != 0) {
        *errMsg = "Failed to add socket to epoll.";
        delete event.data.ptr;
        return false;
    }
    return true;
}

bool Reactor::start() {
    traceIn;
    efd = epoll_create1(0);
    if (efd < 0) {
        logger.error("Startup error. Failed to create epoll instance.");
        return false;
    }
    isStarted = true;
    th = thread([this]{
        logger.debug("Service thread started.");
        array<epoll_event, EPOLL_EVENTS_BUFFER_SIZE> events;
        while (1) {
            int numOfEvents = epoll_wait(efd, events.data(), EPOLL_EVENTS_BUFFER_SIZE, EPOLL_WAIT_TIMEOUT);
            if (numOfEvents == -1) {
                logger.error("epoll wait failed.");
                stop();
                continue;
            }
            if (numOfEvents == 0) {
                if (!isStarted && Task::getNumOfTasks() == 0)
                    break;
            }
            for (int i = 0; i < numOfEvents; i++) {
                data_t* data = (data_t*)events[i].data.ptr;
                if (epoll_ctl(efd, EPOLL_CTL_DEL, data->second, nullptr) != 0) {
                    logger.err() << "Error deleting socket from epoll: " << strerror(errno) << endl;
                    continue;
                }
                Task* task = data->first;
                logger.dbg() << "Task " << data->first << " is ready to be resumed. Adding it to the queue." << endl;
                delete data;
                queue.push(task);
            }
        }
        close(efd);
        logger.debug("Thread stopped.");
    });
    return true;
}

void Reactor::stop() {
    traceIn;
    isStarted = false;
    th.join();
}
