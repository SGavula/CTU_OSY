#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

struct Node {
    int x;
    char *text;
    struct Node *next;
};

// Global variables
struct Node *head = NULL;
pthread_mutex_t mutex_list;
pthread_mutex_t mutex_list_read;
sem_t sem;
int num_threads = 1;
int incorrect_input = 0;

/* Declarations of functions */
void append_node(struct Node **head, struct Node *new_node);
void free_linked_list(struct Node *head);
void print_output(int idx, int count, const char* text);
struct Node* create_node();
void* execute_prod();
void* execute_con(void *arg);

/* MAIN */
int main(int argc, char* argv[]) {
    // Handle execution of program
    if(argc > 1) {
        int input_num = atoi(argv[1]);
        int max_cores = sysconf(_SC_NPROCESSORS_ONLN);
        if(input_num <= 0 || input_num > max_cores) {
            perror("Invalid number of threads.");
            return EXIT_FAILURE;
        }
        num_threads = input_num;
    } else if(argc > 2) {
        perror("Invalid number of arguments.");
        return EXIT_FAILURE;
    }

    // Create variable for threads
    pthread_t producer;
    pthread_t consuments[num_threads];
    // Init mutexes and semaphore
    pthread_mutex_init(&mutex_list, NULL);
    pthread_mutex_init(&mutex_list_read, NULL);
    sem_init(&sem, 0, 0);
    
    // Create producer thread 
    if (pthread_create(&producer, NULL, &execute_prod, NULL) != 0) {
        perror("Failed to create thread.");
        return EXIT_FAILURE;
    }

    // Create consuments threads
    for(int i = 0; i < num_threads; i++) {
        int *thread_arg = malloc(sizeof(int));
        if (thread_arg == NULL) {
            perror("Memory allocation failed");
            free_linked_list(head);
            return EXIT_FAILURE;
        }
        *thread_arg = i;
        if (pthread_create(&consuments[i], NULL, &execute_con, thread_arg) != 0) {
            perror("Failed to create thread.");
            free_linked_list(head);
            return EXIT_FAILURE;
        }
    }
    
    // Wait for end of producer
    if (pthread_join(producer, NULL) != 0) {
        perror("Failed to join thread.");
        free_linked_list(head);
        return EXIT_FAILURE;
    }

    // Wait for end of consumers
    for(int i = 0; i < num_threads; i++) {
        if (pthread_join(consuments[i], NULL) != 0) {
            perror("Failed to join thread.");
            free_linked_list(head);
            return EXIT_FAILURE;
        }
    }

    // Deallocte list and destroy mutexes and semaphore
    free_linked_list(head);
    pthread_mutex_destroy(&mutex_list);
    pthread_mutex_destroy(&mutex_list_read);
    sem_destroy(&sem);
    
    // Returned value based on input
    if(incorrect_input == 0) {
        return 0;
    }else {
        return EXIT_FAILURE;
    }
}

/* Definitions of functions */
void append_node(struct Node **head, struct Node *new_node) {
    pthread_mutex_lock(&mutex_list);
    if (*head == NULL) {
        *head = new_node;
    }else {
        struct Node *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    pthread_mutex_unlock(&mutex_list);
}

void free_linked_list(struct Node *head) {
    while (head != NULL) {
        pthread_mutex_lock(&mutex_list);
        struct Node *temp = head;
        head = head->next;
        pthread_mutex_unlock(&mutex_list);
        free(temp->text);
        free(temp);
    }
}

void print_output(int idx, int count, const char* text) {
    printf("Thread %i:", idx);
    for (int i = 0; i < count; i++) {
        printf(" %s", text);
    }
    printf("\n");
}

struct Node* create_node() {
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    if (new_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    new_node->x = 0;
    new_node->text = NULL;
    new_node->next = NULL;
    return new_node;
}

void* execute_prod() {
    int ret;
    struct Node *new_node = create_node();
    while ((ret = scanf("%d %ms", &new_node->x, &new_node->text)) == 2) {
        if(new_node->x < 0) {
            // Signal remaining consumer threads
            for(int i = 0; i < num_threads; i++) {
                sem_post(&sem);
            }
            free(new_node);
            free_linked_list(head);
            exit(EXIT_FAILURE);
        }
        // Append new node to link list
        append_node(&head, new_node);
        sem_post(&sem);

        // Create new node
        new_node = create_node();
    }
    // Signal remaining consumer threads
    for(int i = 0; i < num_threads; i++) {
        sem_post(&sem);
    }
    free(new_node);
    if(ret == 0) {
        incorrect_input = 1;
    }
    return NULL;
}

void* execute_con(void *arg) {
    int idx = *(int*)arg;
    free(arg);

    while(1) {
        // Wait for next element in linked list
        sem_wait(&sem);
        pthread_mutex_lock(&mutex_list_read);
        if (head == NULL) {
            pthread_mutex_unlock(&mutex_list_read);
            break;
        }
        struct Node *node = head;
        head = head->next;
        pthread_mutex_unlock(&mutex_list_read);

        print_output((idx + 1), node->x, node->text);
        
        // Deallocate memory
        free(node->text);
        free(node);
    }
    return NULL;
}