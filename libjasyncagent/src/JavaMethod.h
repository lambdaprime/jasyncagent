#ifndef JAVAMETHOD_H_
#define JAVAMETHOD_H_

#include <string>

#include "jni.h"

class JavaMethod
{

    jclass clazz;
    jmethodID methodId;
    std::string className;
    std::string methodName;
    std::string methodSign;

public:

    JavaMethod();
    JavaMethod(jclass clazz, jmethodID methodId);
    JavaMethod(const std::string& className,
            const std::string& methodName,
            const std::string& methodSign);
    virtual ~JavaMethod();

    const std::string& getClassName() {
        return className;
    }

    const std::string& getMethodName() {
        return methodName;
    }

    const std::string& getMethodSignature() {
        return methodSign;
    }

    jclass getClass() const
    {
        return clazz;
    }

    void setClass(jclass clazz)
    {
        this->clazz = clazz;
    }

    jmethodID getId() const
    {
        return methodId;
    }

    void setId(jmethodID methodId)
    {
        this->methodId = methodId;
    }

    void callVoid(JNIEnv* jniEnv, jobject obj);

    jobject callObject(JNIEnv* jniEnv, jobject obj);

    static bool create(JNIEnv* jniEnv, const std::string& className,
            const std::string& name, const std::string& sign, JavaMethod& out);

};

#endif /* JAVAMETHOD_H_ */
