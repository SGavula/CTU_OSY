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
    int last_work;
    int prev_works[_PRODUCT_COUNT];
    int time;
    pthread_t thread;
};

struct Workplace {
    int queue[_PLACE_COUNT]; // Number of products that should be done on the each workplace, removing after work worked on the product
    int not_done_queue[_PLACE_COUNT]; // Number of products that should be done on the each workplace, removing after product is done
    bool have_work[_PLACE_COUNT]; // Which worker has work to do and which does not
    int num_of_workers[_PLACE_COUNT]; // Number of workers on the each workplace
    int num_of_workplaces[_PLACE_COUNT]; // Number of each workplace, all workplaces = sum(num_of_workplaces)
    int static_num_of_workplaces[_PLACE_COUNT];
    int times[_PLACE_COUNT]; // Time for each workplace 
};

struct Workplace workplace = {
    .queue = {0},
    .not_done_queue = {0},
    .have_work = {false},
    .num_of_workers = {0},
    .num_of_workplaces = {0},
    .static_num_of_workplaces = {0},
    .times = {100, 200, 150, 300, 400, 250, 500},
};

struct WorkplacePositions wp_pos = {
    .indexes = {{0, 2}, {2, 8}, {8, 9}, {9, 10}, {10, 13}, {13, 15}, {15, 18}},
    .pos_in_pt = {{1, 1}, {0, 0}, {0, 4}, {1, 3}, {2, 3}, {0, 1}, {2, 1}, {1, 0}, {0, 2}, {0, 3}, {0, 5}, {2, 5}, {1, 4}, {1, 5}, {2, 2}, {2, 4}, {1, 2}, {2, 0}},
};

// Global variables
pthread_cond_t cond_work_avail;
pthread_mutex_t mutex_work;
int products_table[_PRODUCT_COUNT][_PHASE_COUNT] = {0};
int working_table[_PRODUCT_COUNT][_PHASE_COUNT] = {0};
struct Worker *workers = NULL;
const char *print_table[_PRODUCT_COUNT][_PHASE_COUNT];
bool possible_finish[_PRODUCT_COUNT] = {true, true, true};
int products_table_info[_PRODUCT_COUNT][_PHASE_COUNT] = {
    {NUZKY, VRTACKA, OHYBACKA, SVARECKA, VRTACKA, LAKOVNA},
    {VRTACKA, NUZKY, FREZA, VRTACKA, LAKOVNA, SROUBOVAK},
    {FREZA, VRTACKA, SROUBOVAK, VRTACKA, FREZA, LAKOVNA}
};
volatile int num_of_workers = 0;
volatile bool end_of_app = false;

// Declaration of functions
int find_string_in_array(const char **array, int length, char *what);
void print_products_table();
void print_workplace(struct Workplace workplace);
void print_wp_pos();
void cleanup_memory();
void* custom_realloc();
void sleep_ms(int milliseconds);
void increase_queue(int product_id);
void decrease_queue(int product_id);
int check_for_item(int workplace_id, int *x, int *y);
bool is_prev_working(int workplace_id);
void find_worker_by_name(const char *name);
bool is_work_possible(int workplace_id);
void are_products_makeable();
void terminate_product_workers(int product_id);
bool all_products_unmakeable();
void terminate_thread(struct Worker worker);
void init_prev_works(int num_of_workers);
bool worker_can_exit(struct Worker worker, int worker_id);
void working(struct Worker worker, int worker_id, int x, int y);
void* worker_function(void *arg);
void handle_end_of_app();
void handle_start(const char *name, int workplace_id);
void handle_make(int product_id);
void handle_add(int workplace_id);
void handle_remove(int workplace_id);

/* MAIN */
int main(int argc, char **argv)
{
    /* Initialize your internal structures, mutexes and condition variables.
     */
    pthread_mutex_init(&mutex_work, NULL);
    pthread_cond_init(&cond_work_avail, NULL);
    while (1) {
        char *line, *cmd, *arg1, *arg2, *arg3, *saveptr;
        int s = scanf(" %m[^\n]", &line);
        if (s == EOF) {
            handle_end_of_app();
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
                handle_start(arg1, workplace_id);
            } else {
                fprintf(stderr, "Invalid workplace name: %s\n", arg2);
            }
        } else if (strcmp(cmd, "make") == 0 && arg1 && !arg2) {
            int product_id = find_string_in_array(product_str, _PRODUCT_COUNT, arg1);
            if(product_id >= 0) {
                handle_make(product_id);
            }else {
                fprintf(stderr, "Invalid product name: %s\n", arg1);
            }
        } else if (strcmp(cmd, "end") == 0 && arg1 && !arg2) {
            // find worker by name and tell the worker to finish
            find_worker_by_name(arg1);
        } else if (strcmp(cmd, "add") == 0 && arg1 && !arg2) {
            // add new place if worker and part is ready, start working - wakeup worker
            int workplace_id = find_string_in_array(place_str, _PLACE_COUNT, arg1);
            if(workplace_id >= 0) {
                handle_add(workplace_id);
            }else {
                fprintf(stderr, "Invalid workplace: %s\n", arg1);
            }
        } else if (strcmp(cmd, "remove") == 0 && arg1 && !arg2) {
            /* if you cannot remove empty place you cannot wait for finish
             * work
             */
            int workplace_id = find_string_in_array(place_str, _PLACE_COUNT, arg1);
            if(workplace_id >= 0) {
                handle_remove(workplace_id);
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
    // Clean memory, destroy mutexes and conditional variable
    cleanup_memory();
    pthread_mutex_destroy(&mutex_work);
    pthread_cond_destroy(&cond_work_avail);
    return 0;
}


// Definitions of functions
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

    printf("\nNum of static Workplaces: ");
    for (int i = 0; i < _PLACE_COUNT; i++) {
        printf("%d ", workplace.static_num_of_workplaces[i]);
    }

    printf("\n");
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
    for(int i = 0; i < _PHASE_COUNT; i++) {
        int workplace_id = products_table_info[product_id][i];
        workplace.queue[workplace_id]++;
        workplace.not_done_queue[workplace_id]++;
        workplace.have_work[workplace_id] = true;
    }
}

void decrease_queue(int product_id) {
    for(int i = 0; i < _PHASE_COUNT; i++) {
        int workplace_id = products_table_info[product_id][i];
        workplace.not_done_queue[workplace_id]--;
    }
}

int check_for_item(int workplace_id, int *x, int *y) {
    int indexes_temp[2];
    memcpy(indexes_temp, wp_pos.indexes[workplace_id], sizeof(indexes_temp));
    int start = indexes_temp[0];
    int end = indexes_temp[1];
    for(int i = start; i < end; i++) {
        int x_temp = wp_pos.pos_in_pt[i][0];
        int y_temp = wp_pos.pos_in_pt[i][1];
        if(products_table[x_temp][y_temp] > 0) {
            *x = x_temp;
            *y = y_temp;
            return 0;
        }
    }

    return 1;
}

bool is_prev_working(int workplace_id) {
    int indexes_temp[2];
    memcpy(indexes_temp, wp_pos.indexes[workplace_id], sizeof(indexes_temp));
    int start = indexes_temp[0];
    int end = indexes_temp[1];
    for(int i = start; i < end; i++) {
        int x_temp = wp_pos.pos_in_pt[i][0];
        int y_temp = wp_pos.pos_in_pt[i][1] - 1; 
        if(y_temp >= 0) {
            if(working_table[x_temp][y_temp] > 0) {
                return true;
            }
        }
    }

    return false;
}

void find_worker_by_name(const char *name) {
    for (int i = 0; i < num_of_workers; ++i) {
        pthread_mutex_lock(&mutex_work);
        if (strcmp(workers[i].name, name) == 0) {
            workers[i].exit = true;
        }
        pthread_mutex_unlock(&mutex_work);
    }
}

bool is_work_possible(int workplace_id) {
    int indexes_temp[2];
    memcpy(indexes_temp, wp_pos.indexes[workplace_id], sizeof(indexes_temp));
    int start = indexes_temp[0];
    int end = indexes_temp[1];
    for(int i = start; i < end; i++) {
        int x_temp = wp_pos.pos_in_pt[i][0];
        int y_temp = wp_pos.pos_in_pt[i][1] - 1;
        if(y_temp >= 0) {
            int prev_workplace_id = products_table_info[x_temp][y_temp];
            // Skip workplaces that do not have any product to do
            if(workplace.queue[prev_workplace_id] <= 0) {
                continue;
            }

            // Return true if there is some worker in previous stage or if workplace is present
            if(workplace.num_of_workers[prev_workplace_id] != 0 && workplace.static_num_of_workplaces[prev_workplace_id] != 0) {
                return true;
            }
        }
    }

    return false;
}

void are_products_makeable() {
    for(int i = 0; i < _PRODUCT_COUNT; i++) {
        // Assumption that product is makeable
        int res = true; 
        for(int j = 0; j < _PHASE_COUNT; j++) {
            int workplace_id = products_table_info[i][j];
            if(workplace.static_num_of_workplaces[workplace_id] == 0 || workplace.num_of_workers[workplace_id] == 0) {
                // Product is makeable
                res = false;
                break;
            }
        }
        possible_finish[i] = res;
    }
}

void terminate_product_workers(int product_id) {
    decrease_queue(product_id);

    for(int i = 0; i < num_of_workers; i++) {
        int workplace_id = workers[i].workplace;
        for(int j = 0; j < _PHASE_COUNT; j++) {
            if(workplace_id == products_table_info[product_id][j]) {
                workers[i].prev_works[product_id]--;
            }
        }

        // Assume that all products that worker worked on are finished
        bool res = true; 
        for(int j = 0; j < _PRODUCT_COUNT; j++) {
            if(workers[i].prev_works[j] > 0) {
                // Some product that worker worked on it is not finished
                res = false;
                break;
            }
        }

        if(res == true && workplace.not_done_queue[workers[i].workplace] == 0) {
            workers[i].went_home = true;
        }
    }
}

bool all_products_unmakeable() {
    bool result = true;
    for(int i = 0; i < _PRODUCT_COUNT; i++) {
        if(possible_finish[i] == true) {
            // Some product can be made
            result = false;
            break;
        }
    }
    return result;
}

void terminate_thread(struct Worker worker) {
    workplace.num_of_workers[worker.workplace]--;
    printf("%s goes home\n", worker.name);
    pthread_cond_broadcast(&cond_work_avail);
    pthread_mutex_unlock(&mutex_work);
}

void init_prev_works(int num_of_workers) {
    for (int i = 0; i < _PRODUCT_COUNT; i++) {
        workers[num_of_workers].prev_works[i] = 0;
    }
}

bool worker_can_exit(struct Worker worker, int worker_id) {
    // Terminate worker after end command is called
    if(workers[worker_id].exit == true) {
        terminate_thread(worker);
        return true;
    }

    if(end_of_app == true && is_prev_working(worker.workplace) == false) {
        // Nobody cant work
        if(all_products_unmakeable() == true && is_work_possible(worker.workplace) == false) {
            terminate_thread(worker);
            return true;
        }
        // Exit worker that has no work to do
        if(workplace.have_work[worker.workplace] == false) {
            terminate_thread(worker);
            return true; 
        }

        //Exit worker that has some work to do but the product is unmakeable
        if(is_work_possible(worker.workplace) == false && worker.last_work >= 0 && possible_finish[worker.last_work] == false) {
            terminate_thread(worker);
            return true;
        }

        // Terminate worker that worked on some product after product is done
        if(workplace.queue[worker.workplace] == 0 || is_work_possible(worker.workplace) == false) {
            if(workers[worker_id].went_home == true) {
                terminate_thread(worker);
                return true;
            }
        }
    }
    return false;
}

void working(struct Worker worker, int worker_id, int x, int y) {
    printf("%s %s %d %s\n", worker.name, place_str[worker.workplace], (y+1), product_str[x]);
    workers[worker_id].last_work = x;
    workers[worker_id].prev_works[x]++;
    workplace.num_of_workplaces[worker.workplace]--;
    workplace.queue[worker.workplace]--;
    products_table[x][y]--;
    working_table[x][y]++;
    if(y+1 == _PHASE_COUNT) {
        printf("done %s\n", product_str[x]);
    }
    pthread_mutex_unlock(&mutex_work);
}

void work_done(struct Worker worker, int x, int y) {
    pthread_mutex_lock(&mutex_work);
    if((y+1) < _PHASE_COUNT) {
        products_table[x][y+1]++;
        working_table[x][y]--;
    }else {
        terminate_product_workers(x);
    }
    workplace.num_of_workplaces[worker.workplace]++;
    pthread_cond_broadcast(&cond_work_avail);
    pthread_mutex_unlock(&mutex_work);
}

void* worker_function(void *arg) {
    int worker_id = *(int*)arg;
    free(arg);

    // Value of product that can be made
    int x = 0;
    // Value of phase where product can be made
    int y = 0;
    
    while (1) {
        pthread_mutex_lock(&mutex_work);
        struct Worker worker = workers[worker_id];
        // Worker must have workplace and product
        while (workplace.num_of_workplaces[worker.workplace] == 0 || check_for_item(worker.workplace, &x, &y) == 1) {
            if(worker_can_exit(worker, worker_id) == true) {
                return NULL;
            }
            pthread_cond_wait(&cond_work_avail, &mutex_work);
        }

        working(worker, worker_id, x, y);
        sleep_ms(worker.time);
        work_done(worker, x, y);
    }
}

void handle_end_of_app() {
    pthread_mutex_lock(&mutex_work);
    end_of_app = true;
    //Check if products can be made
    are_products_makeable();
    pthread_mutex_unlock(&mutex_work);
    pthread_cond_broadcast(&cond_work_avail);
}

void handle_start(const char *name, int workplace_id) {
    pthread_mutex_lock(&mutex_work);
    workplace.num_of_workers[workplace_id]++;
    workers = custom_realloc();
    // Copy information about worker to the struct
    workers[num_of_workers].name = strdup(name);
    workers[num_of_workers].workplace = workplace_id;
    workers[num_of_workers].exit = false;
    workers[num_of_workers].went_home = false;
    workers[num_of_workers].time = workplace.times[workplace_id];
    workers[num_of_workers].last_work = -1; 
    init_prev_works(num_of_workers);
    pthread_mutex_unlock(&mutex_work);
    int *thread_arg = malloc(sizeof(int));
    if (thread_arg == NULL) {
        perror("Memory allocation failed");
        // Free existing memory
        cleanup_memory();
        exit(EXIT_FAILURE);
    }
    *thread_arg = num_of_workers;
    pthread_mutex_lock(&mutex_work);
    // Start worker thread
    if (pthread_create(&workers[num_of_workers].thread, NULL, worker_function, thread_arg) != 0) {
        // Free existing memory
        cleanup_memory();
        perror("Failed to create thread.");
        pthread_mutex_unlock(&mutex_work);
        exit(EXIT_FAILURE);
    }
    num_of_workers++;
    pthread_mutex_unlock(&mutex_work);
}

void handle_make(int product_id) {
    pthread_mutex_lock(&mutex_work);
    products_table[product_id][0]++;
    increase_queue(product_id);
    are_products_makeable();
    pthread_mutex_unlock(&mutex_work);
    pthread_cond_broadcast(&cond_work_avail);
}

void handle_add(int workplace_id) {
    pthread_mutex_lock(&mutex_work);
    workplace.num_of_workplaces[workplace_id]++; 
    workplace.static_num_of_workplaces[workplace_id]++;
    pthread_mutex_unlock(&mutex_work);
}

void handle_remove(int workplace_id) {
    pthread_mutex_lock(&mutex_work);
    workplace.num_of_workplaces[workplace_id]--;
    workplace.static_num_of_workplaces[workplace_id]--;
    pthread_mutex_unlock(&mutex_work);
}