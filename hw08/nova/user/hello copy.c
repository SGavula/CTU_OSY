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

    addr0 = my_malloc(0x80000);
    addr1 = my_malloc(0x80000);
    addr2 = my_malloc(0x80000);
    addr3 = my_malloc(0x80000);
    addr4 = my_malloc(0x80000);
    addr5 = my_malloc(0x80000);
    addr6 = my_malloc(0x80000);
    addr7 = my_malloc(0x80000);
    addr8 = my_malloc(0x80000);
    addr9 = my_malloc(0x80000);
    addr10 = my_malloc(0x80000);
    addr11 = my_malloc(0x80000);
    addr12 = my_malloc(0x80000);
    addr13 = my_malloc(0x80000);
    addr14 = my_malloc(0x80000);
    addr15 = my_malloc(0x80000);
    addr16 = my_malloc(0x80000);
    addr17 = my_malloc(0x80000);
    addr18 = my_malloc(0x80000);
    addr19 = my_malloc(0x80000);
    addr20 = my_malloc(0x80000);
    addr21 = my_malloc(0x80000);
    addr22 = my_malloc(0x80000);
    addr23 = my_malloc(0x80000);
    addr24 = my_malloc(0x80000);
    addr25 = my_malloc(0x80000);
    addr26 = my_malloc(0x80000);
    addr27 = my_malloc(0x80000);
    addr28 = my_malloc(0x80000);
    addr29 = my_malloc(0x80000);
    addr30 = my_malloc(0x80000);
    addr31 = my_malloc(0x80000);
    addr32 = my_malloc(0x80000);
    addr33 = my_malloc(0x80000);
    addr34 = my_malloc(0x80000);
    addr35 = my_malloc(0x80000);
    my_free(addr1);
    my_free(addr2);
    my_free(addr3);
    my_free(addr8);
    my_free(addr9);
    my_free(addr10);
    my_free(addr11);
    my_free(addr16);
    my_free(addr17);
    my_free(addr18);
    my_free(addr19);
    my_free(addr24);
    my_free(addr25);
    my_free(addr26);
    my_free(addr27);
    my_free(addr32);
    my_free(addr33);
    my_free(addr34);
    my_free(addr4);
    my_free(addr5);
    my_free(addr6);
    my_free(addr7);
    my_free(addr12);
    my_free(addr13);
    my_free(addr14);
    my_free(addr15);
    my_free(addr20);
    my_free(addr21);
    my_free(addr22);
    my_free(addr23);
    my_free(addr28);
    my_free(addr29);
    my_free(addr30);
    my_free(addr31);
    addr1 = my_malloc(0x100000);
    printf("New address 1: %#lx\n", addr1);
    addr2 = my_malloc(0x100000);
    printf("New address 2: %#lx\n", addr2);
    addr3 = my_malloc(0x100000);
    printf("New address 3: %#lx\n", addr3);

    while (1) ;
}
