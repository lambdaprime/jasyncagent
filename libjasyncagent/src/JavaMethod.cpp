#include "JavaMethod.h"
#include "Logger.h"

using namespace std;

static Logger logger("JavaMethod");

JavaMethod::JavaMethod()
    : clazz(nullptr), methodId(0)
{

}

JavaMethod::~JavaMethod()
{

}

JavaMethod::JavaMethod(jclass clazz, jmethodID methodId)
    : clazz(clazz), methodId(methodId)
{

}

JavaMethod::JavaMethod(const string& className,
            const string& methodName,
            const string& methodSign)
    : clazz(nullptr), methodId(0), className(className), methodName(methodName), methodSign(methodSign)
{

}

jclass findClass(JNIEnv* jniEnv, const string& className) {
    jclass testclass = jniEnv->FindClass(className.c_str());
    if (testclass == nullptr)
    {
        logger.dbg() << "class not found: " << className << endl;
        return nullptr;
    }
    return testclass;
}

jmethodID findMethodId(JNIEnv* jniEnv, jclass clazz, const string& methodName, const string& methodSignature) {
    jmethodID methodId = jniEnv->GetMethodID(clazz, methodName.c_str(), methodSignature.c_str());
    if (methodId == nullptr)
    {
        logger.dbg() << "method not found: " << methodName << endl;
        return nullptr;
    }
    return methodId;
}

bool JavaMethod::create(JNIEnv* jniEnv, const string& className,
        const string& name, const string& sign, JavaMethod& out)
{
    traceIn;
    jclass clazz = findClass(jniEnv, className.c_str());
    if (clazz == nullptr)
        return false;
    jmethodID methodId = findMethodId(jniEnv, clazz, name.c_str(), sign.c_str());
    if (methodId == nullptr)
        return false;
    out = JavaMethod(clazz, methodId);
    return true;
}

void JavaMethod::callVoid(JNIEnv* jniEnv, jobject obj) {
    return jniEnv->CallVoidMethod(obj, methodId);
}

jobject JavaMethod::callObject(JNIEnv* jniEnv, jobject obj) {
    return jniEnv->CallObjectMethod(obj, methodId);
}

