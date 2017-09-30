#include <dlfcn.h>
#include <errno.h>
#include <err.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <pthread.h>

#include <iostream>

#include "Message.h"

#define PREFIX "[jasyncagentpreload] "

#ifdef DEBUG

void _trace(const char *format, ...) {
    va_list(arglist);
    va_start(arglist, format);
    vfprintf(stdout, format, arglist);
}

#define trace(...) \
    _trace("%s %s (%d): ", PREFIX, __FILE__, __LINE__); \
    _trace(__VA_ARGS__); \
    fprintf(stdout, "\n")

#else

#define trace(...)

#endif

bool init();

static bool isPreloadSuccessful = init();
static int id;
static Message* msg = nullptr;

typedef int (*pthread_attr_getstack_t)(const pthread_attr_t *__restrict __attr,
        void **__restrict __stackaddr,
        size_t *__restrict __stacksize);

typedef int (*pthread_key_create_t)(pthread_key_t *key, dtor_t);

typedef void* (*pthread_getspecific_t)(pthread_key_t tlsKey);

typedef int (*pthread_setspecific_t)(pthread_key_t tlsKey, const void *value);

static pthread_attr_getstack_t pthread_attr_getstack_orig = nullptr;
static pthread_key_create_t pthread_key_create_orig = nullptr;
static pthread_getspecific_t pthread_getspecific_orig = nullptr;
static pthread_setspecific_t pthread_setspecific_orig = nullptr;

Message* createMessage() {
    id = shmget(IPC_PRIVATE, sizeof(Message), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (id == -1) {
        perror(0);
        return nullptr;
    }
    void* addr = shmat(id, 0, 0);
    if (addr == (void*)-1) {
        perror(0);
        return nullptr;
    }
    trace("id %d", id);
    int len = 20;
    char str[len + 1];
    snprintf(str, len, "%d", id);
    setenv(MESSAGE_ID_VAR, str, true);
    memset(addr, 0, sizeof(Message));
    return (Message*)addr;
}

void* findSymbol(const char* name) {
    void* sym = dlsym(RTLD_NEXT, name);
    if (dlerror() != nullptr)
        return nullptr;
    return sym;
}

bool init() {
    msg = createMessage();
    if (!msg) {
        errx(1, "%sFailed to create a message", PREFIX);
        msg->isPreloadSuccessful = false;
        return false;
    }
    pthread_attr_getstack_orig = (pthread_attr_getstack_t)findSymbol("pthread_attr_getstack");
    if (!pthread_attr_getstack_orig) {
        errx(1, "%sCannot find symbol pthread_attr_getstack", PREFIX);
        msg->isPreloadSuccessful = false;
        return false;
    }
    pthread_key_create_orig = (pthread_key_create_t)findSymbol("pthread_key_create");
    if (!pthread_key_create_orig) {
        errx(1, "%sCannot find symbol pthread_key_create", PREFIX);
        msg->isPreloadSuccessful = false;
        return false;
    }
    pthread_getspecific_orig = (pthread_getspecific_t)findSymbol("pthread_getspecific");
    if (!pthread_getspecific_orig) {
        errx(1, "%sCannot find symbol pthread_getspecific", PREFIX);
        msg->isPreloadSuccessful = false;
        return false;
    }
    pthread_setspecific_orig = (pthread_setspecific_t)findSymbol("pthread_setspecific");
    if (!pthread_setspecific_orig) {
        errx(1, "%sCannot find symbol pthread_setspecific", PREFIX);
        msg->isPreloadSuccessful = false;
        return false;
    }
    msg->isPreloadSuccessful = true;
    return true;
}

void __attribute__ ((destructor)) cleanup() {
    trace("Cleaning up the message");
    shmdt(msg);
    if (shmctl(id, IPC_RMID, 0) == -1)
    {
        perror("Failed to release memory allocated for the message");
        return;
    }
}

bool shouldIgnore() {
    return !msg->isCoroutineActive || !pthread_equal(pthread_self(), msg->id);
}

int pthread_attr_getstack(const pthread_attr_t *__restrict __attr,
                  void **__restrict __stackaddr,
                  size_t *__restrict __stacksize)
{
    trace("Intercepted pthread_attr_getstack");
    // TODO get rid of these checks
    if (!isPreloadSuccessful)
        return ENOTSUP;
    if (shouldIgnore())
        return pthread_attr_getstack_orig(__attr, __stackaddr, __stacksize);
    trace("Substituting values");
    *__stackaddr = msg->stackAddr;
    *__stacksize = msg->stackSize;
    return 0;
}

int pthread_key_create(pthread_key_t *key, dtor_t dtor)
{
    trace("Intercepted pthread_key_create");
    if (!isPreloadSuccessful)
        return ENOTSUP;
    int ret = pthread_key_create_orig(key, dtor);
    if (!shouldIgnore())
        msg->tsm->addKey(*key, dtor);
    return ret;
}

void* pthread_getspecific(pthread_key_t key)
{
    //trace("Intercepted pthread_getspecific");
    if (!isPreloadSuccessful)
        return nullptr;
    if (shouldIgnore())
        return pthread_getspecific_orig(key);
    if (!msg->tsm->hasKey(key))
        return nullptr;
    //trace("Substituting values");
    return msg->tsm->getData(key);
}

int pthread_setspecific(pthread_key_t key, const void* value)
{
    trace("Intercepted pthread_setspecific");
    if (!isPreloadSuccessful)
        return ENOTSUP;
    if (shouldIgnore())
        return pthread_setspecific_orig(key, value);
    msg->tsm->setData(key, (void*)value);
    return 0;
}
