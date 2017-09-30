#include <iostream>
#include <ucontext.h>


using namespace std;

ucontext_t maincontext;
ucontext_t childcontext;
bool isChildReady;

int corochild() {
    isChildReady = true;
    cout << "corochild1" << endl;
    getcontext(&childcontext);
    swapcontext(&childcontext, &maincontext);
    cout << "corochild2" << endl;
    swapcontext(&childcontext, &maincontext);
}

int coromain() {
    cout << "coromain1" << endl;
    getcontext(&maincontext);
    if (!isChildReady)
        corochild();
    cout << "coromain2" << endl;
    swapcontext(&maincontext, &childcontext);
    cout << "coromain3" << endl;
}

int main(int argc, char** argv) {
    coromain();
}

