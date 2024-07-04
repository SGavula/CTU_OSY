#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "volatile.c"

void f() {
    static int cout = 0;
    cout = cout + 1;
    printf("A: %d\n", cout);
}

int main() {
    f();
    f();
    return 0;
}