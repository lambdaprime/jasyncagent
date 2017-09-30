#include <JavaMethod.h>
#include <memory>
#include <vector>

#include "Logger.h"
#include "RunnableCaller.h"

using namespace std;

static Logger logger("RunnableCaller");

RunnableCaller::RunnableCaller(jobject obj)
    : obj(obj)
{

}

RunnableCaller::~RunnableCaller() {

}

enum Method {
    RUNNABLE_RUN,
    THROWABLE_TOSTRING,
    THROWABLE_PRINTSTACKTRACE,
    SIZE
};

vector<JavaMethod> javaMethods(SIZE);

string getException(JNIEnv* jniEnv) {
    jthrowable exception = jniEnv->ExceptionOccurred();
    jniEnv->ExceptionClear();
    jstring msgObj = static_cast<jstring>(javaMethods[THROWABLE_TOSTRING].
            callObject(jniEnv, exception));
    if (msgObj == nullptr) {
        logger.err() << "Failed to get exception" << endl;
        return "";
    }
    const char* msg = jniEnv->GetStringUTFChars(msgObj, 0);
    string s(msg);
    jniEnv->ReleaseStringUTFChars(msgObj, msg);
    jniEnv->DeleteLocalRef(msgObj);

    javaMethods[THROWABLE_PRINTSTACKTRACE].callVoid(jniEnv, exception);

    return s;
}

void RunnableCaller::call(JNIEnv* jniEnv) {
    traceIn;
    javaMethods[RUNNABLE_RUN].callVoid(jniEnv, obj);
    if (jniEnv->ExceptionCheck()) {
        logger.err() << "Task throwed unhandled exception: " << endl;
        string msg = getException(jniEnv);
        if (!msg.empty())
            logger.err() << msg << endl;
        return;
    }
}

bool RunnableCaller::init(JNIEnv* jniEnv) {
    traceIn;
    if (!JavaMethod::create(jniEnv, "java/lang/Runnable", "run",
            "()V", javaMethods[RUNNABLE_RUN]))
        return false;
    if (!JavaMethod::create(jniEnv, "java/lang/Throwable", "toString",
            "()Ljava/lang/String;", javaMethods[THROWABLE_TOSTRING]))
        return false;
    if (!JavaMethod::create(jniEnv, "java/lang/Throwable", "printStackTrace",
            "()V", javaMethods[THROWABLE_PRINTSTACKTRACE]))
        return false;
    return true;
}


