#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>

static int counter = 0;

// This function simulates an external event updating 'counter'
void* updateCounter(void* arg) {
    while (1) {
        counter++;
        sleep(1); // Wait for 1 second
    }
}

int main() {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, updateCounter, NULL);

    while (1) {
        printf("Counter value: %d\n", counter);
        sleep(1); // Wait for 1 second
    }

    return 0;
}