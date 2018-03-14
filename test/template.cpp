#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

void kprintf(const char *format, ...) {
    printf(format);
}

void ASSERT(bool expr, string msg)
{
    if (!expr) {
        cout << "\nERROR: " << msg << "\n";
        exit(3);
    }
}

void run_func(string func_name, void (*func)()) {
    cout << "  " << func_name << "\n";
    func();
}

int error = true;

void end_of_tests(void) {
    if (error)
        cout << "\nError running tests\n";
}


//@test-include

void main(void)
{
    atexit(&end_of_tests);
    //@test-functions
    error = false;
    cout << "\n\nSuccess\n";
}
