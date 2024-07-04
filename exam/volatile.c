#include <stdio.h>
#include <stdlib.h>

volatile int c = 0;

int main() {
    c = c + 1;
    printf("C: %d\n", c);
    return 0;
}