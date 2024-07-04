#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

// Define a struct to store the data
struct Node {
    int x;
    char *text;
    struct Node *next;
};

// Global variables
struct Node *head = NULL;
pthread_mutex_t mutex_list;
pthread_mutex_t mutex_print;
sem_t sem;
volatile int num_threads = 1;
volatile int incorrect_input = 0;

// Function to add a new node to the end of the linked list
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

// Function to free memory allocated for the linked list
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

void* execute_prod() {
    int ret;
    struct Node *new_node; 
    new_node = (struct Node*)malloc(sizeof(struct Node));
    if (new_node == NULL) {
        perror("Memory allocation failed");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    new_node->next = NULL;
    while ((ret = scanf("%d %ms", &new_node->x, &new_node->text)) == 2) {
        if(new_node->x < 0) {
            for(int i = 0; i < num_threads; i++) {
                sem_post(&sem); // Signal remaining consumer threads
            }
            free(new_node);
            free_linked_list(head);
            exit(EXIT_FAILURE);
        }
        // Append new node to link list
        append_node(&head, new_node);
        sem_post(&sem);

        // Create new node
        new_node = (struct Node*)malloc(sizeof(struct Node));
        if (new_node == NULL) {
            perror("Memory allocation failed");
            free(new_node);
            free_linked_list(head);
            exit(EXIT_FAILURE);
        }
        new_node->x = 0;
        new_node->text = NULL;
        new_node->next = NULL;
    }
    for(int i = 0; i < num_threads; i++) {
        sem_post(&sem); // Signal remaining consumer threads
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
        sem_wait(&sem);
        pthread_mutex_lock(&mutex_list);
        if (head == NULL) {
            pthread_mutex_unlock(&mutex_list);
            break;
        }
        struct Node *node = head;
        head = head->next;
        pthread_mutex_unlock(&mutex_list);

        // Print the output as described
        pthread_mutex_lock(&mutex_print);
        printf("Thread %i:", (idx + 1));
        for (int i = 0; i < node->x; i++) {
            printf(" %s", node->text);
        }
        printf("\n");
        pthread_mutex_unlock(&mutex_print);

        free(node->text);
        free(node);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    // Handle execution of program
    if(argc > 1) {
        int input_num = atoi(argv[1]);
        int max_cores = sysconf(_SC_NPROCESSORS_ONLN);
        if(input_num <= 0 || input_num > max_cores) {
            perror("Invalid number of threads.");
            return 1;
        }
        num_threads = input_num;
    } else if(argc > 2) {
        perror("Invalid number of arguments.");
        return 1;
    }

    // Create variable for threads
    pthread_t producer;
    pthread_t consuments[num_threads];
    pthread_mutex_init(&mutex_list, NULL);
    pthread_mutex_init(&mutex_print, NULL);
    sem_init(&sem, 0, 0);
    
    // Create producer thread 
    if (pthread_create(&producer, NULL, &execute_prod, NULL) != 0) {
        perror("Failed to create thread.");
        return 1;
    }

    // Create consuments threads
    for(int i = 0; i < num_threads; i++) {
        int *thread_arg = malloc(sizeof(int));
        if (thread_arg == NULL) {
            perror("Memory allocation failed");
            free_linked_list(head);
            return 1;
        }
        *thread_arg = i;
        if (pthread_create(&consuments[i], NULL, &execute_con, thread_arg) != 0) {
            perror("Failed to create thread.");
            free_linked_list(head);
            return 1;
        }
    }
    
    // Wait for end of producer
    if (pthread_join(producer, NULL) != 0) {
        perror("Failed to join thread.");
        free_linked_list(head);
        return 1;
    }

    // Wait for end of consumers
    for(int i = 0; i < num_threads; i++) {
        if (pthread_join(consuments[i], NULL) != 0) {
            perror("Failed to join thread.");
            free_linked_list(head);
            return 1;
        }
    }

    free_linked_list(head);
    pthread_mutex_destroy(&mutex_list);
    pthread_mutex_destroy(&mutex_print);
    sem_destroy(&sem);
    
    if(incorrect_input == 0) {
        return 0;
    }else {
        return 1;
    }
}