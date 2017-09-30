#ifndef ASYNCAGENT_H_
#define ASYNCAGENT_H_

#include <string>

#include "Queue.h"
#include "Task.h"

extern Queue<Task*> queue;

void throwException(JNIEnv *jniEnv, const std::string& msg);

#endif /* ASYNCAGENT_H_ */
