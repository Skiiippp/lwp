#include <lwp.h>
//#include <stdio.h>
#include <sys/mman.h>

/* REMOVE FOR SUBMISSION & TESTING - ONLY FOR LINTER*/
//#define MAP_ANONYMOUS 0
// #define MAP_STACK 0

void test(void *farted);

int main()
{
    char *farted = "FARTED";
    void *arg = (void *)farted;

    unsigned long *stack = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    stack[0] = 42;   // just generic so we don't go out of bounds
    stack[1] = (unsigned long)((uintptr_t)test); // RA
    stack[2] = 0;   // base pointer and stack pointer

    rfile curr;
    swap_rfiles(&curr, NULL);

    rfile regs;
    regs.rsp = (unsigned long)((uintptr_t)stack + 2); 
    regs.rbp = regs.rsp;
    regs.rax = (unsigned long)((uintptr_t)arg);
    regs.fxsave = curr.fxsave;

    swap_rfiles(NULL, &regs);

    return 0;
}


void test(void *farted)
{
    char *out = (char *)farted;
    //printf("Test string: %s\n", out);
    exit(1);
}