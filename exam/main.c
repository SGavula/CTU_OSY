#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {

    int f = fork();
    if (f == 0) {
        // Close stdin
        close(0);
        printf("Start asdfasdf\n");
        open("a.txt", O_RDONLY | O_CREAT);
        execl("./prg", "./prg", NULL);
    } else {
        wait(NULL);
    }

    return 0;
}