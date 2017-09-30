#ifndef NATIVECALLSINTERCEPTOR_H_
#define NATIVECALLSINTERCEPTOR_H_

#include <jvmti.h>
#include <string>

namespace NativeCallsInterceptor
{
    using namespace std;

    extern bool init(jvmtiEnv*);

    extern void add(const string& className, const string& methodName, const string& methodSign,
            void* newFunc, void** origFunc);

};

#endif /* NATIVECALLSINTERCEPTOR_H_ */
