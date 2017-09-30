#include <iostream>
#include <pth.h>
#include <unistd.h>

using namespace std;

static void* child(void*) {
    cout << "child 1" << endl;
    pth_yield(nullptr);
    cout << "child 2" << endl;
}

int main(int argc, char** argv) {
    pth_init();

    pth_attr_t attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_NAME, "ticker");
    pth_attr_set(attr, PTH_ATTR_STACK_SIZE, 64*1024);
    pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);

    cout << "main 1" << endl;
    //if (pth_spawn(attr, child, nullptr) == nullptr)
    //    cout << "spawn error" << endl;
    child(nullptr);
    //pth_yield(nullptr);
    cout << "main 2" << endl;
    child(nullptr);
    //pth_yield(nullptr);
    //sleep(3);
    while (1);
}

