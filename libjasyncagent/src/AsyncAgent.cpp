#include "AsyncAgent.h"
#include "Logger.h"

static Logger logger("AsyncAgent");

Queue<Task*> queue;

using namespace std;

void throwException(JNIEnv* jniEnv, const string& msg)
{
    const char* className = "id/asyncagent/JAsyncAgentException";
    jclass clazz = jniEnv->FindClass(className);
    if (clazz == NULL) {
        logger.err() << msg << endl;
        return;
    }
    jniEnv->ThrowNew(clazz, msg.c_str());
}
