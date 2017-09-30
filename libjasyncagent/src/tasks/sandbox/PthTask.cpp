#include <Queue.h>
#include <iostream>

#include "PthTask.h"

using namespace std;

PthTask::PthTask(jobject obj) :
    Task(obj),
    pth(nullptr)
{
    static auto isPthInited = pth_init();
}

extern JavaVM* jvm;

void* runInternal_(PthTask* task) {
    cout << "-task enter" << endl;
    JNIEnv* looperEnv;
    if (jvm->AttachCurrentThread((void**)&looperEnv, NULL) != JNI_OK) {
        cerr << "cannot attach looper thread to JVM" << endl;
    }
    task->callRunnable();
    cout << "-task suspended" << endl;
    pth_yield(nullptr);
    cout << "-task was resumed" << endl;
    cout << "-task exit" << endl;
    return (void*)nullptr;
}

int PthTask::run() {
    //runnable->call();
    if (pth == nullptr) {
        pth_attr_t attr = pth_attr_new();
        pth_attr_set(attr, PTH_ATTR_NAME, "ticker");
        pth_attr_set(attr, PTH_ATTR_STACK_SIZE, 64000*1024);
//        pth_attr_set(attr, PTH_ATTR_STACK_ADDR, &attr + 5000);
        pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);

        pth = pth_spawn(attr, reinterpret_cast<void*(*)(void*)>(runInternal_), this);
        if (pth == nullptr)
            cout << "spawn error" << endl;
    }
//    runnable->call();
    pth_yield(pth);

    /*
    cout << "-task started" << std::this_thread::get_id() << endl;
    JNIEnv* looperEnv;
    if (jvm->AttachCurrentThread((void**)&looperEnv, NULL) != JNI_OK) {
        cerr << "cannot attach looper thread to JVM" << endl;
    }
    runnable->setJniEnv(looperEnv);
    //this->sink = &sink;
    // Runnable.call
    runnable->call();
    /*

            queue.push(this);
            sink(123);

            //sink(100);
     */
    //cout << "-task finishing" << endl;

    return -1;
}
