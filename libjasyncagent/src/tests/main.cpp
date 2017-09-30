#include <list>
#include <thread>
#include <unistd.h>

using namespace std;

#include "Logger.h"

void test_Logger() {
    Logger l("Logger");
    l << 10 << " is the number" << endl;
    l << "ping pong" << endl;
    l.info("hello");
    l.err() << "Error" << endl;
}

#include "tasks/UContextTask.h"

void UContextTask() {
    char t[1024];
    t[0] = 'a';
    t[1] = 'b';
    cout << sizeof(char) << endl;
    cout << &t << endl;
    cout << (t + 1) << endl;
    cout << &t[0] << endl;
    cout << &t[1] << endl;
    return;
    list<Task*> l;
    for (int i = 0; i < 5000; ++i) {
        l.push_back(UContextTask::create(nullptr).release());
    }

    for (Task* t: l) {
        cout << t << endl;
        delete t;
    }

    list<thread> lt;
    for (int i = 0; i < 5000; ++i) {
        lt.push_back(thread([]{
                Task * t = UContextTask::create(nullptr).release();
                cout << "del" << endl;
                sleep(1);
                delete t;
        }));
    }

    while(1);
    cout << "asdfasdf" << endl;
}


#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>

static ucontext_t uctx_main, uctx_func1;

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

void f() {
    char ch1[10];
    printf("%p\n" , &ch1);
    char ch2;
    printf("%p\n" , &ch2);
}

static void
func1(void)
{
    printf("func1: started\n");
    printf("func1: swapcontext(&uctx_func1, &uctx_func2)\n");
    char ch1;
    printf("%p\n" , &ch1);
    char ch2;
    printf("%p\n" , &ch2);
    f();
    printf("func1: returning\n");
}

void test_UContextTask() {
    char func1_stack[16384];
    printf("%p\n" , &func1_stack);
    if (getcontext(&uctx_func1) == -1)
        handle_error("getcontext");
    uctx_func1.uc_stack.ss_sp = func1_stack;
    uctx_func1.uc_stack.ss_size = sizeof(func1_stack);
    uctx_func1.uc_link = &uctx_main;
    makecontext(&uctx_func1, func1, 0);

    printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
    if (swapcontext(&uctx_main, &uctx_func1) == -1)
        handle_error("swapcontext");

    printf("main: exiting\n");
    exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[])
{

}

