#include <atomic>

#include "Queue.h"
#include "Task.h"
#include "Logger.h"
#include "tasks/UContextTask.h"

using namespace std;

static Logger logger("Task");

static std::atomic<int> numOfTasks(0);

Task::Task(jobject obj) :
    runnable(new RunnableCaller(obj)),
    obj(obj),
    jniEnv(nullptr),
    hasCompleted(false)
{
    numOfTasks++;
    logger.dbg() << "Number of tasks " << std::dec << numOfTasks << endl;
}

Task::~Task()
{
    traceIn;
    numOfTasks--;
    logger.dbg() << "Number of tasks " << std::dec << numOfTasks << endl;
}

void Task::callRunnable() {
    runnable->call(jniEnv);
}

bool Task::isCompleted() {
    return hasCompleted;
}

void Task::finish() {
    traceIn;
    hasCompleted = true;
    jniEnv->DeleteGlobalRef(obj);
    if (jvm->DetachCurrentThread() != JNI_OK) {
        logger.error("Cannot detach task thread from JVM.");
        return;
    }
    jniEnv = nullptr;
    logger.dbg() << "Task " << this << " finished." << endl;
}

int Task::run() {
    traceIn;
    runInternal();
    if (hasCompleted) {
        logger.dbg() << "Task " << this << " completed." << endl;
        return -1;
    }
    return 0;
}

void Task::runInternal(Task* task) {
    traceIn;
    logger.dbg() << "Task " << task << " started." << endl;
    if (jvm->AttachCurrentThread((void**)&task->jniEnv, NULL) != JNI_OK) {
        logger.error("Cannot attach task thread to JVM. Aborting the task.");
        task->finish();
        return;
    }
    task->callRunnable();
    task->finish();
}

void Task::yield() const {
    traceIn;
    logger.dbg() << "Task " << this << " suspended." << endl;
    yieldInternal();
    logger.dbg() << "Task " << this << " resumed." << endl;
}

Task* Task::create(JNIEnv* jniEnv, jobject obj) {
    traceIn;
    obj = jniEnv->NewGlobalRef(obj);
    if (obj == nullptr)
        return nullptr;
    return UContextTask::create(obj).release();
}

bool Task::setup() {
    return UContextTask::initialize();
}

int Task::getNumOfTasks() {
    return numOfTasks;
}
