#include "stdio.h"
#include "mem_alloc.h"
#include "string.h"

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

int main ()
{
    printf("Hello world asdfasdfasdf!\n");

    printf("Variables test: uninitialized_var=%d initialized_var=%d\n",
           uninitialized_var, initialized_var);
    void *b = nbrk(0);
    b = nbrk(0x0);  
    printf("current break: %#lx\n", b);
    b = nbrk(0x4000);   
    printf("current break: %#lx\n", b);
    b = nbrk(0x6000);    
    printf("current break: %#lx\n", b);
    b = nbrk(0x15001);    
    printf("current break: %#lx\n", b);
    b = nbrk(0xc0000000);    
    printf("current break: %#lx\n", b);
    b = nbrk(0xdeadbeef);   
    printf("current break: %#lx\n", b);
    b = nbrk(0x0);
    printf("current break: %#lx\n", b);
    b = nbrk(0xd000);    
    printf("current break: %#lx\n", b);
    b = nbrk(0x1d800);    
    printf("current break: %#lx\n", b);
    b = nbrk(0x1234);
    printf("current break: %#lx\n", b);
    b = nbrk(0x0);
    printf("current break: %#lx\n", b);
    b = nbrk(0x2000);
    printf("current break: %#lx\n", b);
    b = nbrk(0x1e000);    
    printf("current break: %#lx\n", b);
    b = nbrk(0x1d800);   
    printf("current break: %#lx\n", b);
    b = nbrk(0x1f222);    
    printf("current break: %#lx\n", b);
    b = nbrk(0x2000000);
    printf("current break: %#lx\n", b);
    // printf("current break: %#lx\n", b);
    // // nbrk((void*)(0x2fff));
    // b = nbrk(0x4000);
    // b = nbrk(0);
    // printf("new break: %#lx\n", b);
    // printf("#############\n");
    // nbrk((void*)(0x7555));
    // b = nbrk(0);
    // printf("new break: %#lx\n", b);
    // printf("#############\n");
    // nbrk((void*)(0x7666));
    // b = nbrk(0);
    // printf("new break: %#lx\n", b);
    // printf("#############\n");
    // nbrk((void*)(0xA000));
    // b = nbrk(0);
    // printf("new break: %#lx\n", b);

    while (1) ;
}
