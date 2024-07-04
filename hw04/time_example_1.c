#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

sem_t mutex;

void* thread(void* arg)
{
    //wait
    printf("Hello this is thread.\n");
}

// Pri threadoch na case zavisi
// Vytvorenie a volanie thread funkcie pri vytvarani trva nejaky cas preto main thread dalej pokracuje vo vykonavani svojho programu a vypisuje Hello this is main thread
int main()
{
    // sem_init(&mutex, 0, 1);
    pthread_t t1,t2;
    pthread_create(&t1,NULL,thread,NULL);
    for(int i = 0; i < 100000; i++) {
        printf("Hello this is main thread.\n");
    }

    // sleep(2);
    pthread_create(&t2,NULL,thread,NULL);
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    // sem_destroy(&mutex);
    return 0;
}
