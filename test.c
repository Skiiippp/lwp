#include <lwp.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdlib.h>

#define STACK_SIZE 8*1000*1024
#define BYTES_PER_WORD 8

/* REMOVE FOR SUBMISSION & TESTING - ONLY FOR LINTER*/
#define MAP_ANONYMOUS 0
#define MAP_STACK 0

void test(void *farted);

int main()
{
    char *farted = "FARTED";
    void *arg = (void *)farted;

    unsigned long *stack = (unsigned long *)mmap(NULL, STACK_SIZE * BYTES_PER_WORD, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    void (*test_func)(void *farted) = test;

    /*
    stack[0] = 0x12345678;   // just generic so we don't go out of bounds
    //stack[1] = (unsigned long)((uintptr_t)test_func); // RA
    stack[1] = 0x87654321;
    stack[2] = 0x9999999;   // base pointer and stack pointer
    */
    // STACK GROWS UPSIDE DOWN
    stack[STACK_SIZE - 2] = (unsigned long)((uintptr_t)test_func);  // RA
    stack[STACK_SIZE - 3] = (unsigned long)((uintptr_t)&stack[STACK_SIZE-1]);   // BP


    rfile regs;
    swap_rfiles(&regs, NULL);

    //printf("Test func ptr: %p\n", (void *)(uintptr_t)stack[STACK_SIZE - 1]);
    //fflush(stdout);

    regs.rsp = (unsigned long)((uintptr_t)&stack[STACK_SIZE-3]); 
    regs.rbp = regs.rsp;
    regs.rdi = (unsigned long)((uintptr_t)arg);


    swap_rfiles(NULL, &regs);

    return 0;
}

void test(void *farted)
{
    char *out = (char *)farted;
    printf("Test string: %s\n", out);

    exit(0);
}