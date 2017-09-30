#include <iostream>

#include <fcntl.h>

#include "jni.h"
#include "jvmti.h"

#include "Task.h"
#include "Looper.h"
#include "Logger.h"
#include "Reactor.h"
#include "RunnableCaller.h"
#include "NativeCallsInterceptor.h"
#include "Message.h"
#include "AsyncAgent.h"

extern "C" {

JNIEXPORT void JNICALL Java_id_asyncagent_JAsyncAgent_addRunnable
    (JNIEnv*, jclass, jobject);
JNIEXPORT void JNICALL Java_id_asyncagent_JAsyncAgent_start
    (JNIEnv*, jclass);
JNIEXPORT void JNICALL Java_id_asyncagent_JAsyncAgent_stop
    (JNIEnv*, jclass);

JNIEXPORT jint JNICALL newSocketRead(
    JNIEnv *jni_env, jclass clazz, jobject fd, jbyteArray b, jint off, jint len,
    jint timeout);
JNIEXPORT jint JNICALL newSocketWrite(
    JNIEnv *jni_env, jclass clazz, jobject fd, jbyteArray b, jint off, jint len);

}

using namespace std;

JavaVM* jvm;

static Looper looper;
static Reactor reactor;

static jint (* origSocketRead)(
    JNIEnv *jni_env, jclass clazz, jobject fd, jbyteArray b, jint off, jint len,
    jint timeout);
static jint (* origSocketWrite)(
    JNIEnv *jni_env, jclass clazz, jobject fd, jbyteArray b, jint off, jint len);

static Logger logger("JAsyncAgentJni");
static jfieldID fdFieldId;

static volatile bool isStarted = false;

JNIEXPORT void JNICALL Java_id_asyncagent_JAsyncAgent_start
  (JNIEnv *jniEnv, jclass clazz)
{
    looper.start(jvm);
    if (!reactor.start()) {
        throwException(jniEnv, "Failed to start Reactor");
        return;
    }

    if (!RunnableCaller::init(jniEnv)) {
        throwException(jniEnv, "Java caller initialization failed");
        return;
    }

    const string& className = "java/io/FileDescriptor";
    jclass fdclazz = jniEnv->FindClass(className.c_str());
    if (fdclazz == nullptr)
    {
        throwException(jniEnv, "Class " + className + " not found");
        return;
    }

    const string& fieldName = "fd";
    fdFieldId = jniEnv->GetFieldID(fdclazz, fieldName.c_str(), "I");
    if (fdFieldId == nullptr)
    {
        throwException(jniEnv, "Field " + fieldName + " not found");
        return;
    }

    isStarted = true;
}

JNIEXPORT void JNICALL Java_id_asyncagent_JAsyncAgent_stop
  (JNIEnv *jni_env, jclass clazz)
{
    traceIn;
    isStarted = false;
    reactor.stop();
    looper.stop();
}

JNIEXPORT void JNICALL Java_id_asyncagent_JAsyncAgent_addRunnable
  (JNIEnv* jniEnv, jclass clazz, jobject obj)
{
    if (!isStarted)
        return;
    Task* task = Task::create(jniEnv, obj);
    if (task == nullptr) {
        throwException(jniEnv, "Failed to create the task");
        return;
    }
    logger.dbg() << "Adding Runnable (" << task << ") to the queue" << endl;
    queue.push(task);
    return;
}

JNIEXPORT jint JNICALL newSocketRead(
    JNIEnv* jniEnv, jclass clazz, jobject fd, jbyteArray b, jint off, jint len,
    jint timeout)
{
    if (!looper.isOnLooperThread())
        return origSocketRead(jniEnv, clazz, fd, b, off, len, timeout);
    if (!isStarted && Task::getNumOfTasks() == 0)
        return origSocketRead(jniEnv, clazz, fd, b, off, len, timeout);
    logger.dbg() << "socketRead intercepted" << endl;
    jint socket = jniEnv->GetIntField(fd, fdFieldId);
    Task* task = looper.getCurrentTask();
    const char* err = nullptr;
    if (!reactor.add(socket, task, Reactor::READ, &err)) {
        throwException(jniEnv, err);
        return origSocketRead(jniEnv, clazz, fd, b, off, len, timeout);
    }
    task->yield();
    return origSocketRead(jniEnv, clazz, fd, b, off, len, timeout);
}

JNIEXPORT jint JNICALL newSocketWrite(
    JNIEnv* jniEnv, jclass clazz, jobject fd, jbyteArray b, jint off, jint len)
{
    if (!looper.isOnLooperThread())
        return origSocketWrite(jniEnv, clazz, fd, b, off, len);
    if (!isStarted && Task::getNumOfTasks() == 0)
        return origSocketWrite(jniEnv, clazz, fd, b, off, len);
    logger.dbg() << "socketWrite intercepted" << endl;
    jint socket = jniEnv->GetIntField(fd, fdFieldId);
    Task* task = looper.getCurrentTask();
    const char* err = nullptr;
    if (!reactor.add(socket, task, Reactor::WRITE, &err)) {
        throwException(jniEnv, err);
        return origSocketWrite(jniEnv, clazz, fd, b, off, len);
    }
    task->yield();
    return origSocketWrite(jniEnv, clazz, fd, b, off, len);
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
    logger.dbg() << "Agent_OnLoad" << endl;
    ::jvm = jvm;

    jvmtiEnv *jvmti = NULL;

    jvm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_0);

    if (!NativeCallsInterceptor::init(jvmti)) {
        logger.err() << "Failed to init interceptor" << endl;
        return JNI_ERR;
    }
    NativeCallsInterceptor::add("java.net.SocketInputStream", "socketRead0",
            "(Ljava/io/FileDescriptor;[BIII)I", (void*)newSocketRead, (void**)&origSocketRead);
    NativeCallsInterceptor::add("java.net.SocketOutputStream", "socketWrite0",
            "(Ljava/io/FileDescriptor;[BII)V", (void*)newSocketWrite, (void**)&origSocketWrite);

    if (!Task::setup()) {
        logger.err() << "Failed to setup the tasks" << endl;
        return JNI_ERR;
    }

    return JNI_OK;
}

