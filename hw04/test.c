#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

sem_t mutex;
pthread_mutex_t mutex_list;

void* execute_con(void* arg)
{   
    int idx = *(int*)arg;
    free(arg);
    printf("Thread with index: %d\n", idx);
}

// Pri threadoch na case zavisi
// Vytvorenie a volanie thread funkcie pri vytvarani trva nejaky cas preto main thread dalej pokracuje vo vykonavani svojho programu a vypisuje Hello this is main thread
int main()
{
    pthread_t consuments[5];;
    for(int i = 0; i < 5; i++) {
        int *thread_arg = malloc(sizeof(int));
        if (thread_arg == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        *thread_arg = i;
        if (pthread_create(&consuments[i], NULL, &execute_con, thread_arg) != 0) {
            perror("Failed to create thread.");
            return 1;
        }
    }

    for(int i = 0; i < 5; i++) {
        if (pthread_join(consuments[i], NULL) != 0) {
            perror("Failed to join thread.");
            return 1;
        }
    }
    return 0;
}



