#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>

#include "Logger.h"
#include "RunnableCaller.h"
#include "UContextTask.h"
#include "Logger.h"

// including stack guard page
#define STACK_SIZE 9027584

using namespace std;

static Logger logger("UContextTask");
static Message* message = nullptr;
static size_t pageSize = getpagesize();

Message* obtainMessage() {
    int id = atoi(getenv(MESSAGE_ID_VAR));
    logger.dbg() << "message id " << id << endl;
    void* addr = shmat(id, 0, 0);
    if (addr == (void*)-1)
        return nullptr;
    return (Message*)addr;
}

bool UContextTask::initialize() {
    if ((message = obtainMessage()) == nullptr) {
        logger.err() << "Failed to obtain message from libjasyncagentpreload library" << endl;
        return false;
    }
    return true;
}

unique_ptr<UContextTask> UContextTask::create(jobject obj) {
    void* stack;
    //if (posix_memalign(&stack, pageSize, STACK_SIZE) != 0)
    //    return nullptr;
    //stack = memalign(pageSize, STACK_SIZE);
    //if (stack == nullptr)
    //        return nullptr;
    stack = mmap(nullptr, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (stack == MAP_FAILED)
        return nullptr;
    // guard stack page
    if (mprotect(stack, pageSize, PROT_NONE) != 0)
        return nullptr;
    logger.dbg() << "stack after: " << stack << endl;
    return unique_ptr<UContextTask>(new UContextTask(obj, stack));
}

UContextTask::UContextTask(jobject obj, void* stack):
    Task(obj),
    stack(stack)
{
    traceIn;
    logger.dbg() << "Task stack addr: " << stack << endl;
}

UContextTask::~UContextTask() {
    traceIn;
    logger.dbg() << "Destroying task " << this << endl;
    logger.dbg() << "Releasing stack " << stack << endl;
    //free(stack);
    munmap(stack, STACK_SIZE);
}

void UContextTask::yieldInternal() const {
    swapcontext(childContext.get(), mainContext.get());
}

void printCurrentStackAddr() {
    pthread_attr_t attr;
    int ret = pthread_getattr_np(pthread_self(), &attr);
    if (ret != 0) {
        logger.error("Fail getting attr");
        return;
    }
    void* addr;
    size_t size;
    if (pthread_attr_getstack(&attr, &addr, &size) != 0) {
        logger.error("Error getting stack addr");
        return;
    }
    logger.dbg() << "current stack addr " << addr << endl;
    logger.dbg() << "stack size " << size << endl;
    char ch;
    logger.dbg() << "current stack addr " << (void*)&ch << endl;
    char ch2;
    logger.dbg() << "current stack addr " << (void*)&ch2 << endl;
}

void runInternal_(UContextTask* task) {
    printCurrentStackAddr();
    Task::runInternal(task);
}

void UContextTask::beforeSwap() {
    message->id = pthread_self();
    message->stackAddr = stack + pageSize;
    message->stackSize = STACK_SIZE - pageSize;
    message->tsm = &manager;
    message->isCoroutineActive = true;
}

void UContextTask::afterSwap() {
    message->isCoroutineActive = false;
    message->tsm = nullptr;
}

void UContextTask::runInternal() {
    if (!mainContext) {
        childContext = unique_ptr<ucontext_t>(new ucontext_t);
        getcontext(childContext.get());
        mainContext = unique_ptr<ucontext_t>(new ucontext_t);
        getcontext(mainContext.get());
        childContext->uc_stack.ss_sp = stack + pageSize;
        childContext->uc_stack.ss_size = STACK_SIZE - pageSize;
        childContext->uc_link = mainContext.get();
        makecontext(childContext.get(), (void (*)())runInternal_, 1, this);
    }
    printCurrentStackAddr();
    beforeSwap();
    swapcontext(mainContext.get(), childContext.get());
    afterSwap();
}
