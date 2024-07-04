#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>

/* You can use these functions and data structures to transform strings to
 * numbers and use them in arrays
 */
#define _PHASE_COUNT 6

enum place {
    NUZKY, VRTACKA, OHYBACKA, SVARECKA, LAKOVNA, SROUBOVAK, FREZA,
    _PLACE_COUNT
};

const char *place_str[_PLACE_COUNT] = {
    [NUZKY] = "nuzky",
    [VRTACKA] = "vrtacka",
    [OHYBACKA] = "ohybacka",
    [SVARECKA] = "svarecka",
    [LAKOVNA] = "lakovna",
    [SROUBOVAK] = "sroubovak",
    [FREZA] = "freza",
};

enum product {
    A, B, C,
    _PRODUCT_COUNT
};

const char *product_str[_PRODUCT_COUNT] = {
    [A] = "A",
    [B] = "B",
    [C] = "C",
};

// Structs
struct WorkplacePositions {
    int indexes[_PLACE_COUNT][2];
    int pos_in_pt[18][2];
};

struct Worker {
    char *name;
    int workplace;
    bool exit;
    bool went_home;
    pthread_t thread;
};

struct Workplace {
    int queue[_PLACE_COUNT]; // Number of products that should be done on the each workplace
    int num_of_workers[_PLACE_COUNT]; // Number of workers on the each workplace
    int num_of_workplaces[_PLACE_COUNT]; // Number of each workplace, all workplaces = sum(num_of_workplaces)
    int times[_PLACE_COUNT];
};

// Global variables
pthread_mutex_t mutex_print;
pthread_cond_t cond_work_avail;
pthread_mutex_t mutex_work;
int products_table[_PRODUCT_COUNT][_PHASE_COUNT] = {0};
struct Worker *workers = NULL;
int num_of_workers = 0;
const char *print_table[_PRODUCT_COUNT][_PHASE_COUNT];
bool end_of_app = false;
int products_table_info[_PRODUCT_COUNT][_PHASE_COUNT] = {
    {NUZKY, VRTACKA, OHYBACKA, SVARECKA, VRTACKA, LAKOVNA},
    {VRTACKA, NUZKY, FREZA, VRTACKA, LAKOVNA, SROUBOVAK},
    {FREZA, VRTACKA, SROUBOVAK, VRTACKA, FREZA, LAKOVNA}
};
struct Workplace workplace = {
    .queue = {0},
    .num_of_workers = {0},
    .num_of_workplaces = {0},
    .times = {100, 100, 150, 300, 400, 250, 500},
};
struct WorkplacePositions wp_pos = {
    .indexes = {{0, 2}, {2, 8}, {8, 9}, {9, 10}, {10, 13}, {13, 15}, {15, 18}},
    .pos_in_pt = {{1, 1}, {0, 0}, {0, 4}, {1, 3}, {2, 3}, {0, 1}, {2, 1}, {1, 0}, {0, 2}, {0, 3}, {0, 5}, {2, 5}, {1, 4}, {1, 5}, {2, 2}, {2, 4}, {1, 2}, {2, 0}},
};

// Functions
int find_string_in_array(const char **array, int length, char *what)
{
    for (int i = 0; i < length; i++)
        if (strcmp(array[i], what) == 0)
            return i;
    return -1;
}

void print_products_table() {
    printf("Products table: \n");
    for (int i = 0; i < _PRODUCT_COUNT; i++) {
        for (int j = 0; j < _PHASE_COUNT; j++) {
            printf("%d ", products_table[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_workplace(struct Workplace workplace) {
    printf("Items that should be made (queue): ");
    for (int i = 0; i < _PLACE_COUNT; i++) {
        printf("%d ", workplace.queue[i]);
    }

    printf("\nNum of Workers: ");
    for (int i = 0; i < _PLACE_COUNT; i++) {
        printf("%d ", workplace.num_of_workers[i]);
    }

    printf("\nNum of Workplaces: ");
    for (int i = 0; i < _PLACE_COUNT; i++) {
        printf("%d ", workplace.num_of_workplaces[i]);
    }

    printf("\n");
}

void cleanup_memory() {
    for (int i = 0; i < num_of_workers; i++) {
        free(workers[i].name);
    }
    free(workers);
}

void* custom_realloc() {
    void *new_ptr = realloc(workers, (num_of_workers + 1) * sizeof(struct Worker));
    
    // Check if realloc was successful
    if (new_ptr == NULL) {
        // Handle realloc failure, for example, by freeing existing memory and exiting the program
        fprintf(stderr, "Memory reallocation failed\n");
        // Free existing memory
        cleanup_memory();
        // Exit the program with an error code
        exit(EXIT_FAILURE);
    }

    return new_ptr;
}

void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

void increase_queue(int product_id) {
    switch (product_id) {
        case 0:
            // Code for product_id 0
            workplace.queue[NUZKY]++;
            workplace.queue[VRTACKA]++;
            workplace.queue[OHYBACKA]++;
            workplace.queue[SVARECKA]++;
            workplace.queue[VRTACKA]++;
            workplace.queue[LAKOVNA]++;
            break;
        case 1:
            // Code for product_id 1
            workplace.queue[VRTACKA]++;
            workplace.queue[NUZKY]++;
            workplace.queue[FREZA]++;
            workplace.queue[VRTACKA]++;
            workplace.queue[LAKOVNA]++;
            workplace.queue[SROUBOVAK]++;
            break;
        default:
            // Code for all other cases
            workplace.queue[FREZA]++;
            workplace.queue[VRTACKA]++;
            workplace.queue[SROUBOVAK]++;
            workplace.queue[VRTACKA]++;
            workplace.queue[FREZA]++;
            workplace.queue[LAKOVNA]++;
            break;
    }
}

int check_for_item(int workplace_id, int *x, int *y) {
    int indexes_temp[2];
    // printf("Workplace: %d\n", workplace_id);
    memcpy(indexes_temp, wp_pos.indexes[workplace_id], sizeof(indexes_temp));
    // printf("HEllo from check: %d\n", workplace_id);
    // print_products_table();
    int start = indexes_temp[0];
    int end = indexes_temp[1];
    // printf("Check for item hello. %d %d\n", start, end);
    for(int i = start; i < end; i++) {
        int x_temp = wp_pos.pos_in_pt[i][0];
        int y_temp = wp_pos.pos_in_pt[i][1];
        // printf("X: Y: %d %d\n", x_temp, y_temp);
        // printf("Value: %d\n", products_table[x_temp][y_temp]);
        if(products_table[x_temp][y_temp] > 0) {
            // printf("X Y in check for item function: %d %d\n", x_temp, y_temp);
            *x = x_temp;
            *y = y_temp;
            return 0;
        }
    }

    return 1;
}

void print_wp_pos() {
    for(int i = 0; i < _PLACE_COUNT; i++) {
        int start = wp_pos.indexes[i][0];
        int end = wp_pos.indexes[i][1];
        for(int j = start; j < end; j++) {
            int x = wp_pos.pos_in_pt[j][0];
            int y = wp_pos.pos_in_pt[j][1];
            printf("%d %d\n", x, y);
            print_table[x][y] = place_str[i];
        }
    }

    printf("Phase table: \n");
    for(int i = 0; i < _PRODUCT_COUNT; i++) {
        for(int j = 0; j < _PHASE_COUNT; j++) {
            printf("%s ", print_table[i][j]);
        }
        printf("\n");
    } 
}

void find_worker_by_name(const char *name) {
    for (int i = 0; i < num_of_workers; ++i) {
        if (strcmp(workers[i].name, name) == 0) {
            workers[i].exit = true;
        }
    }
}

bool is_work_possible(int workplace_id) {
    int indexes_temp[2];
    // printf("Workplace: %d\n", workplace_id);
    memcpy(indexes_temp, wp_pos.indexes[workplace_id], sizeof(indexes_temp));
    // printf("HEllo from check: %d\n", workplace_id);
    // print_products_table();
    int start = indexes_temp[0];
    int end = indexes_temp[1];
    // printf("Check for item hello. %d %d\n", start, end);
    for(int i = start; i < end; i++) {
        int x_temp = wp_pos.pos_in_pt[i][0];
        int y_temp = wp_pos.pos_in_pt[i][1] - 1;
        if(y_temp > 0) {
            
            // int prev_workplace_id = products_table_info[x_temp][y_temp];
            // if(workplace.queue[prev_workplace_id] <= 0) {
            //     continue;
            // }
            // // printf("X: Y: %d %d\n", x_temp, y_temp);
            // // printf("Value: %d\n", products_table[x_temp][y_temp]);
            // if(workplace.num_of_workers[prev_workplace_id] == 0 || workplace.num_of_workplaces[prev_workplace_id] == 0) {
            //     return false;
            // }
        }
    }

    return true;
}

void print_workers_on_completion() {
    for(int i = 0; i < num_of_workers; i++) {
        // Print all workers that went home and change it to default state for other printing
        if(workers[i].went_home == true) {
           printf("%s goes home\n", workers[i].name);
           workers[i].went_home = false;
        }
    }
}

void* worker_function(void *arg) {
    int worker_id = *(int*)arg;
    free(arg);

    int x = 0;
    int y = 0;
    while (1) {
        pthread_mutex_lock(&mutex_work);
        struct Worker worker = workers[worker_id];
        // Worker must have workplace and product
        while (workplace.num_of_workplaces[worker.workplace] == 0 || check_for_item(worker.workplace, &x, &y) == 1) {
            // printf("X value %d\n", x);
            if(worker.exit == true) {
                workplace.num_of_workers[worker.workplace]--;
                printf("%s goes home\n", worker.name);
                pthread_mutex_unlock(&mutex_work);
                return NULL;
            }

            if(end_of_app == true) {
                // printf("Workplace queue: %d\n", y);
                // If worker is unable to work
                printf("workplace.queue[y]: %d\n", workplace.queue[y]);
                if(workplace.queue[y] == 0 && is_work_possible(worker.workplace) == false) {
                    workplace.num_of_workers[worker.workplace]--;
                    printf("%s goes home\n", worker.name);
                    pthread_mutex_unlock(&mutex_work);
                    return NULL;
                }
                // printf("Name of worker: %s | %d\n", worker.name, workplace.queue[worker.workplace]);
                // print_workplace(workplace);
                if(workplace.queue[worker.workplace] == 0 || is_work_possible(worker.workplace) == false) {
                    workplace.num_of_workers[worker.workplace]--;
                    workers[worker_id].went_home = true;
                    // printf("%s goes home\n", worker.name);
                    pthread_mutex_unlock(&mutex_work);
                    return NULL;
                }
            }
            // printf("Worker name: %s waiting\n", worker.name);
            // printf("No products or no workplace.\n");
            // printf("\n");
            pthread_cond_wait(&cond_work_avail, &mutex_work);
        }
        pthread_mutex_lock(&mutex_print);
        // printf("Worker function workplaces: %d checking: %d\n", workplace.num_of_workplaces[worker.workplace], check_for_item(worker.workplace, &x, &y));
        printf("%s %s %d %s\n", worker.name, place_str[worker.workplace], (y+1), product_str[x]);
        // printf("Product table in worker function: \n");
        workplace.num_of_workplaces[worker.workplace]--;
        workplace.queue[worker.workplace]--;
        products_table[x][y]--;
        
        sleep_ms(workplace.times[worker.workplace]);
        if((y+1) < _PHASE_COUNT) {
            products_table[x][y+1]++;
        }else {
            printf("done %s\n", product_str[x]);
            print_workers_on_completion();
        }
        // print_products_table();
        // printf("Signaling.\n\n");
        pthread_mutex_unlock(&mutex_print);
        workplace.num_of_workplaces[worker.workplace]++;
        pthread_cond_broadcast(&cond_work_avail);
        pthread_mutex_unlock(&mutex_work);
    }
    return NULL;
}

/* It is not necessary to represent each working place with a dynamically
 * allocated object.  You can store only number of ready places
 *
 * int ready_places[_PLACE_COUNT];
 */

int main(int argc, char **argv)
{
    /* Initialize your internal structures, mutexes and conddition variables.
     */
    pthread_mutex_init(&mutex_print, NULL);
    pthread_mutex_init(&mutex_work, NULL);
    pthread_cond_init(&cond_work_avail, NULL);
    // print_wp_pos();
    // print_workplace(workplace);
    while (1) {
        char *line, *cmd, *arg1, *arg2, *arg3, *saveptr;
        int s = scanf(" %m[^\n]", &line);
        if (s == EOF) {
            end_of_app = true;
            pthread_cond_broadcast(&cond_work_avail);
            break;
        }

        if (s == 0) continue;

        cmd  = strtok_r(line, " ", &saveptr);
        arg1 = strtok_r(NULL, " ", &saveptr);
        arg2 = strtok_r(NULL, " ", &saveptr);
        arg3 = strtok_r(NULL, " ", &saveptr);

        if (strcmp(cmd, "start") == 0 && arg1 && arg2 && !arg3) {
            /* - start new thread for new worker
             * - copy (e.g. strdup()) worker name from arg1, the
             *   arg1 will be removed at the end of scanf cycle
             * - workers should have dynamic objects, you don't know
             *   total number of workers
             */
            int workplace_id = find_string_in_array(place_str, _PLACE_COUNT, arg2);
            if(workplace_id >= 0) {
                workplace.num_of_workers[workplace_id]++;
                workers = custom_realloc();
                // Copy information about worker to the struct
                workers[num_of_workers].name = strdup(arg1);
                workers[num_of_workers].workplace = workplace_id;
                workers[num_of_workers].exit = false;
                workers[num_of_workers].went_home = false;
                int *thread_arg = malloc(sizeof(int));
                if (thread_arg == NULL) {
                    perror("Memory allocation failed");
                    // Free existing memory
                    cleanup_memory();
                    return EXIT_FAILURE;
                }
                *thread_arg = num_of_workers;
                if (pthread_create(&workers[num_of_workers].thread, NULL, worker_function, thread_arg) != 0) {
                    // Free existing memory
                    cleanup_memory();
                    perror("Failed to create thread.");
                    return EXIT_FAILURE;
                }
                num_of_workers++;
            } else {
                fprintf(stderr, "Invalid workplace name: %s\n", arg2);
            }
        } else if (strcmp(cmd, "make") == 0 && arg1 && !arg2) {
            /* int product = find_string_in_array(
             *     product_str,
             *     _PRODUCT_COUNT,
             *     arg1
             * );
             *
             * if (product >= 0) {.....
             *     add the part to factory cycle
             *     you need to wakeup worker to start working if possible
             *     ...
             *     }
             */
            int product_id = find_string_in_array(product_str, _PRODUCT_COUNT, arg1);
            // printf("Product id: %d\n", product_id);
            if(product_id >= 0) {
                products_table[product_id][0]++;
                increase_queue(product_id);
                // printf("SIGNALING FOR STARTING PRODUCTION.\n");
                pthread_cond_broadcast(&cond_work_avail);
            }else {
                fprintf(stderr, "Invalid product name: %s\n", arg1);
            }
        } else if (strcmp(cmd, "end") == 0 && arg1 && !arg2) {
            /* tell the worker to finish
             * the worker has to finish their work first
             * you should not wait here for the worker to finish
             *
             * if the worker is waiting for work
             * you need to wakeup the worker
             */
            find_worker_by_name(arg1);
        } else if (strcmp(cmd, "add") == 0 && arg1 && !arg2) {
            /* add new place
             *
             * if worker and part is ready, start working - wakeup worker
             */
            int workplace_id = find_string_in_array(place_str, _PLACE_COUNT, arg1);
            if(workplace_id >= 0) {
                workplace.num_of_workplaces[workplace_id]++; 
            }else {
                fprintf(stderr, "Invalid workplace: %s\n", arg1);
            }
        } else if (strcmp(cmd, "remove") == 0 && arg1 && !arg2) {
            /* if you cannot remove empty place you cannot wait for finish
             * work
             */
            int workplace_id = find_string_in_array(place_str, _PLACE_COUNT, arg1);
            if(workplace_id >= 0) {
                workplace.num_of_workplaces[workplace_id]--; 
            }else {
                fprintf(stderr, "Invalid workplace: %s\n", arg1);
            }
        } else {
            fprintf(stderr, "Invalid command: %s\n", line);
        }
        free(line);
    }

    /* Wait for every worker to finish their work. Nobody should be able to
     * continue.
     */
    for(int i = 0; i < num_of_workers; i++) {
        if (pthread_join(workers[i].thread, NULL) != 0) {
            // Free existing memory
            cleanup_memory();
            perror("Failed to join thread.");
            return EXIT_FAILURE;
        }
    }
    cleanup_memory();
    // printf("\n\n");
    // print_products_table();
    // print_workplace(workplace);
    // Destroy mutexes and conditional variable
    pthread_mutex_destroy(&mutex_print);
    pthread_mutex_destroy(&mutex_work);
    pthread_cond_destroy(&cond_work_avail);
    return 0;
}