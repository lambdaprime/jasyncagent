#include "jni.h"

extern "C" {

JNIEXPORT void JNICALL Java_agent_Test_addRunnable
  (JNIEnv*, jclass, jobject);
JNIEXPORT void JNICALL Java_agent_Test_startLooper
  (JNIEnv*, jclass);
JNIEXPORT void JNICALL Java_agent_Test_stopLooper
  (JNIEnv*, jclass);

}

