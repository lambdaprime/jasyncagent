#include <iostream>

using namespace std;

void printCurrentStackAddr() {
    pthread_attr_t attr;
    int ret = pthread_getattr_np(pthread_self(), &attr);
    if (ret != 0) {
        cout << "Fail getting attr" << endl;
        return;
    }
    void* addr;
    size_t size;
    if (pthread_attr_getstack(&attr, &addr, &size) != 0) {
        cout << "Error getting stack addr" << endl;
        return;
    }
    cout << "STACK ADDR " << addr << endl;
    cout << "STACK SIZE " << size << endl;
}

int main() {
    printCurrentStackAddr();
    return 0;
}
