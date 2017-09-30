
#ifndef RUNNABLECALLER_H_
#define RUNNABLECALLER_H_

#include "jni.h"

class RunnableCaller
{

    jobject obj;

public:

    RunnableCaller(jobject);
    virtual ~RunnableCaller();

    void call(JNIEnv* jniEnv);

    static bool init(JNIEnv*);
};

#endif /* RUNNABLECALLER_H_ */
