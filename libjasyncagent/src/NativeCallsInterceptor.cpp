#include <jni.h>

#include <vector>
#include <memory>
#include <algorithm>

#include <memory.h>

#include "NativeCallsInterceptor.h"
#include "Logger.h"
#include "JavaMethod.h"

using namespace std;

struct Method
{
    JavaMethod method;
    void* newFunc;
    void** origFunc;

    Method(const std::string& className,
            const std::string& methodName,
            const std::string& methodSign,
            void* newFunc,
            void** origFunc)
        : method(className, methodName, methodSign), newFunc(newFunc), origFunc(origFunc)
    {

    }
};

static vector<Method> incompletedMethods;
static vector<Method> completedMethods;

static Logger logger("NativeCallsInterceptor");

void JNICALL nativeMethodBind(jvmtiEnv *jvmti_env,
        JNIEnv* jni_env,
        jthread thread,
        jmethodID methodId,
        void* address,
        void** new_address_ptr)
{
    for (auto m: completedMethods) {
        if (m.method.getId() != methodId)
            continue;
        *new_address_ptr = m.newFunc;
        *m.origFunc = address;
        logger.dbg() << "Substituted " << m.method.getMethodName() << " successfully" << endl;
    }
}

string obtainClassName(JNIEnv* jniEnv, jclass clazz)
{

    jclass clsClazz = jniEnv->GetObjectClass(clazz);
    if (clsClazz == nullptr)
        return "";

    jmethodID methodId = jniEnv->GetMethodID(clsClazz, "getName", "()Ljava/lang/String;");
    if (methodId == nullptr)
        return "";

    jstring className = (jstring) jniEnv->CallObjectMethod(clazz, methodId);
    if (className == nullptr)
        return "";

    jint strlen = jniEnv->GetStringUTFLength(className);

    if (strlen == 0) return "";

    unique_ptr<char[]> str(new char[strlen + 1]);
    jniEnv->GetStringUTFRegion(className, 0, strlen, str.get());
    jniEnv->DeleteLocalRef(clsClazz);
    jniEnv->DeleteLocalRef(className);

    return string(str.get());
}

void JNICALL
ClassPrepare(jvmtiEnv *jvmti_env,
        JNIEnv* jniEnv,
        jthread thread,
        jclass clazz)
{
    static Logger logger("NativeCallsInterceptor (JVMTI_EVENT_CLASS_PREPARE handler)");

    if (incompletedMethods.empty())
        return;

    if (jniEnv == nullptr)
    {
        logger.debug("Empty JNIEnv.");
        return;
    }

    const string& className = obtainClassName(jniEnv, clazz);
    if (className.empty()) {
        logger.debug("Failed to obtain class name.");
        return;
    }

    auto it = find_if(incompletedMethods.begin(),
            incompletedMethods.end(),
            [&className](Method& m){
                return m.method.getClassName() == className;
            }
    );

    if (it == incompletedMethods.end())
        return;

    it->method.setClass(clazz);

    jmethodID methodId = jniEnv->GetMethodID(clazz, it->method.getMethodName().c_str(), it->method.getMethodSignature().c_str());
    if (methodId == nullptr) {
        logger.dbg() << "Failed to get " << it->method.getClassName() << " method id " << it->method.getMethodName() << endl;
        return;
    }

    it->method.setId(methodId);

    completedMethods.push_back(*it);
    incompletedMethods.erase(it);

    logger.dbg() << "Loaded " << it->method.getClassName() << "::" << it->method.getMethodName() << endl;

}

bool NativeCallsInterceptor::init(jvmtiEnv* jvmti)
{
    traceIn;

    jvmtiCapabilities capa;
    jvmtiError err;

    err = jvmti->GetPotentialCapabilities(&capa);
    if (err != JVMTI_ERROR_NONE)
    {
        logger.error("Failed to get capabilities.");
        return false;
    }
    err = jvmti->AddCapabilities(&capa);
    if (err != JVMTI_ERROR_NONE)
    {
        logger.error("Failed to add capabilities.");
        return false;
    }

    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.NativeMethodBind = &nativeMethodBind;
    callbacks.ClassPrepare = &ClassPrepare;
    if (jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks)) != JVMTI_ERROR_NONE)
    {
        logger.error("Failed to set event callbacks.");
        return false;
    }
    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, nullptr) != JVMTI_ERROR_NONE)
    {
        logger.error("Failed to enable JVMTI_EVENT_NATIVE_METHOD_BIND.");
        return false;
    }
    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, nullptr) != JVMTI_ERROR_NONE)
    {
        logger.error("Failed to enable JVMTI_EVENT_CLASS_PREPARE.");
        return false;
    }

    return true;
}

void NativeCallsInterceptor::add(
        const string& className,
        const string& methodName,
        const string& methodSign,
        void* newFunc,
        void** origFunc)
{
    traceIn;
    incompletedMethods.push_back({className, methodName, methodSign, newFunc, origFunc});
}

