#include "mem_alloc.h"
#include <stdio.h>
#include <stddef.h>

/*
 * Template for 11malloc. If you want to implement it in C++, rename
 * this file to mem_alloc.cc.
 */

typedef char ALIGN[8];

union header {
    struct {
        size_t size;
        unsigned is_free;
        union header *next;
        union header *prev;
    } s;
    ALIGN stub;
};

typedef union header header_t;

header_t *head, *tail;
int num_of_blocks = 0;

static inline void *nbrk(void *address);

#ifdef NOVA

/**********************************/
/* nbrk() implementation for NOVA */
/**********************************/

static inline unsigned syscall2(unsigned w0, unsigned w1)
{
    asm volatile("   mov %%esp, %%ecx    ;"
                 "   mov $1f, %%edx      ;"
                 "   sysenter            ;"
                 "1:                     ;"
                 : "+a"(w0)
                 : "S"(w1)
                 : "ecx", "edx", "memory");
    return w0;
}

static void *nbrk(void *address)
{
    return (void *)syscall2(3, (unsigned)address);
}
#else

/***********************************/
/* nbrk() implementation for Linux */
/***********************************/

#include <unistd.h>

static void *nbrk(void *address)
{
    void *current_brk = sbrk(0);
    if (address != NULL) {
        int ret = brk(address);
        if (ret == -1)
            return NULL;
    }
    return current_brk;
}

#endif

void print_ll() {
    header_t *tmp = head;
    while(tmp != NULL) {
        // printf("(%d size: %d) -> ", tmp->s.id, tmp->s.size);
        printf("(size: %d) -> ", tmp->s.size);
        tmp = tmp->s.next;
    }
    printf("\n ######## \n");
}

void* get_free_block(unsigned long size) {
    header_t *curr = head;
    while(curr) {
        if(curr->s.is_free == 1 && size <= curr->s.size) {
            if((size + sizeof(header_t)) < curr->s.size) {
                size_t new_size = curr->s.size - size - sizeof(header_t);
                curr->s.size = size;
                // Create new block and put into list
                header_t *header = (header_t*)((char*)curr + size + sizeof(header_t));
                header->s.size = new_size;
                header->s.is_free = 1;
                header->s.next = curr->s.next;
                header->s.prev = curr;
                // Add new block to the 
                curr->s.next = header;
                if(header->s.next != NULL) {
                    header->s.next->s.prev = header;   
                }
            }
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

void *my_malloc(unsigned long size)
{
    /* TODO: Implement in 11malloc */
    size_t total_size;
    void *block;
    header_t *header;

    void *curr_break = nbrk(0);

    // Size is 0
    if(!size) {
        return (void*)0;
    }

    header = get_free_block(size);
    // Block of suitable size exits
    if(header) {
        header->s.is_free = 0;
        return (void*)(header + 1);
    }
    // Creating new block
    total_size = sizeof(header_t) + size;
    block = nbrk(curr_break + total_size);

    if(!block) {
        return (void*)0;
    }
    num_of_blocks++;
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;

    if(!head) {
        head = header;
        header->s.prev = NULL;
    }

    if(tail) {
        header->s.prev = tail;
        tail->s.next = header;
    }

    tail = header;
    return (void*)(header + 1);
}

void do_defragmentation(header_t *header) {
    header_t *tmp_next = header->s.next;

    if(tmp_next != NULL && tmp_next->s.is_free == 1) {
        header->s.next = tmp_next->s.next;
        if(tmp_next->s.next != NULL) {
           tmp_next->s.next->s.prev = header; 
        }
        header->s.size = header->s.size + sizeof(header_t) + tmp_next->s.size;
    }

    header_t *tmp_prev = header->s.prev;
    if(tmp_prev != NULL && tmp_prev->s.is_free == 1) {
        tmp_prev->s.next = header->s.next;
        if(header->s.next != NULL) {
            header->s.next->s.prev = tmp_prev;
        }
        tmp_prev->s.size = tmp_prev->s.size + sizeof(header_t) + header->s.size;
    }
}

int my_free(void *address)
{
    /* TODO: Implement in 11malloc */
    header_t *header, *tmp;
    size_t dealloc_size;

    if(!address) {
        return 1;
    }

    // Get header of the block
    header = (header_t*)address - 1;
    
    // Check for a valid header
    if (header < head || header > tail) {
        printf("Invalid address to free\n");
        return 1;
    }

    // Check if the block is already free
    if (header->s.is_free) {
        printf("Block is already freed\n");
        return 1;
    }
    void *curr_break = nbrk(0);
    if((char*)address + header->s.size == curr_break) {
        // Remove from linked list
        if(head == tail) {
            head = tail = NULL;
            dealloc_size = sizeof(header_t) + header->s.size;
        } else {
            // Size of last block
            dealloc_size = sizeof(header_t) + header->s.size;
            tmp = header->s.prev;
            // Go backwards until hitting the allocated block
            while(tmp->s.is_free != 0) {
                // Save the size of the block
                dealloc_size = dealloc_size + sizeof(header_t) + tmp->s.size;
                if(tmp->s.prev == NULL) {
                    break;
                }
                // Go back
                tmp = tmp->s.prev;
            }
            // Set tail to the last allocated block
            tail = tmp;
            tail->s.next = NULL;
            if(tmp->s.prev == NULL && tmp->s.is_free == 1) {
                head = tail = NULL;
            }
        }
        // Deallocate the memory
        nbrk(curr_break - dealloc_size);
        return 0;
    }

    header->s.is_free = 1;

    // Make defragmentation
    do_defragmentation(header);
    return 0;
}
