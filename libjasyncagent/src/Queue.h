#ifndef QUEUE_H_
#define QUEUE_H_

#include <list>
#include <mutex>
#include <condition_variable>

#include "Logger.h"

template<typename T>
class Queue {

    std::list<T> q;
    std::mutex m;
    std::condition_variable cv;

public:

    void push(const T&);
    void consume(std::function<void(T)>);
    void notify();
    int size();

};

template<typename T>
void Queue<T>::push(const T& e) {
    m.lock();
    traceIn;
    q.push_back(e);
    traceIn;
    m.unlock();
    cv.notify_all();
}

template<typename T>
void Queue<T>::consume(std::function<void(T)> f) {
    std::unique_lock<std::mutex> lk(m);
    if (q.empty())
        cv.wait(lk);
    traceIn;
    T e = q.front();
    q.pop_front();
    traceIn;
    lk.unlock();
    f(e);
}

template<typename T>
void Queue<T>::notify() {
    cv.notify_all();
}

template<typename T>
int Queue<T>::size() {
    return q.size();
}

#endif /* QUEUE_H_ */
