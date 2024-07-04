#include "stdio.h"
#include "mem_alloc.h"
#include "string.h"
#include "stdbool.h"

unsigned syscall2(unsigned w0, unsigned w1)
{
    asm volatile (
        "   mov %%esp, %%ecx    ;"
        "   mov $1f, %%edx      ;"
        "   sysenter            ;"
        "1:                     ;"
        : "+a" (w0) : "S" (w1) : "ecx", "edx", "memory");
    return w0;
}

void *nbrk(void *address)
{
    return (void*)syscall2(3, (unsigned)address);
}

int uninitialized_var;
int initialized_var = 42;

void set_memory_range(void* start, size_t size, unsigned char value) {
}

void *allocValue(unsigned long length, int value)
{
    void *adress = my_malloc(length);
    set_memory_range(adress, length, value);
    return adress;
}

bool testValue(void *adress, unsigned long length, int value)
{
    for (unsigned long i = 0; i < length; i++)
    {
        if (*(char*)(adress + i) != value)
        {
            printf("Error at %p val %d\n", (void*)(adress + i), *(char*)(adress + i));
            return false;
        }
    }
    return true;
}

int main ()
{
    printf("Hello world asdfasdfasdf!\n");

    printf("Variables test: uninitialized_var=%d initialized_var=%d\n",
           uninitialized_var, initialized_var);
    // void *b = nbrk(0);
    // b = nbrk(0x0);  
    // printf("current break: %#lx\n", b);
    // b = nbrk(0x4000);
    // printf("current break: %#lx\n", b);
    // b = nbrk(0x6000);    
    // printf("current break: %#lx\n", b);
    // b = nbrk(0x0);    
    // printf("current break: %#lx\n", b);
    // b = nbrk(0x1000);    
    // printf("current break: %#lx\n", b);
    // void *address = my_malloc(0x8082);
    // printf("current address: %#lx\n", address);
    // int res = my_free(address);
    // printf("Free res: %d\n", res);

    // void *addr = my_malloc(0x400);
    // printf("New address: %#lx\n", addr);
    // int err = 0;
    // err = my_free(0x3010);
    // printf("Error 1: %d\n", err);
    // err = my_free(0x3010);
    // printf("Error 2: %d\n", err);
    // err = my_free(0x2420);
    // printf("Error 3: %d\n", err);

    // int err = 0;
    // void *addr = my_malloc(0x3ff);
    // // Result: addr == 0x3010
    // printf("New address 1: %#lx\n", addr);
    // addr = my_malloc(0x3a);
    // printf("New address 2: %#lx\n", addr);
    // // Result: addr == 0x341f
    // addr = my_malloc(0xc0bd0);
    // printf("New address 3: %#lx\n", addr);
    // // Result: addr == 0x3469

    // err = my_free(0x3010);  
    // printf("Error 1: %d\n", err);
    // err = my_free(0x341f); 
    // printf("Error 2: %d\n", err);
    // err = my_free(0x3469);
    // printf("Error 3: %d\n", err);
    
    unsigned char *addr0, *addr1, *addr2, *addr3, *addr4, *addr5, *addr6, *addr7, *addr8, *addr9;
    unsigned char *addr10, *addr11, *addr12, *addr13, *addr14, *addr15, *addr16, *addr17, *addr18, *addr19;
    unsigned char *addr20, *addr21, *addr22, *addr23, *addr24, *addr25, *addr26, *addr27, *addr28, *addr29;
    unsigned char *addr30, *addr31, *addr32, *addr33, *addr34, *addr35;

    int err = 0;

    addr0 = my_malloc(0x400);
    printf("New address 1: %#lx\n", addr0);
    addr0 = my_malloc(0x400); 
    printf("New address 2: %#lx\n", addr0);
    err = my_free(0x3010);
    printf("Error 1: %d\n", err);
    addr0 = my_malloc(0x3ff);
    printf("New address 3: %#lx\n", addr0);
    err = my_free(0x3420);
    printf("Error 2: %d\n", err);
//     Evaluating: addr = my_malloc(0x400)  
// Evaluating: addr = my_malloc(0x400)  
// Evaluating: err = my_free(0x3014)   
// Evaluating: addr = my_malloc(0x3ff)  
//             Result: addr == 0x3014
// Evaluating: err = my_free(0x3428)   
// Error: Assertion 'err == 0' failed because err == 1 and 0 == 0
// Evaluating: err = my_free(0x3014)

    while (1) ;
}
