#include <lwp.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>

#include "round_robin.h"


/* REMOVE FOR SUBMISSION & TESTING - ONLY FOR LINTER*/
//#define MAP_ANONYMOUS 0
//#define MAP_STACK 0

/* If RLIMIT_STACK is unreasonable - 8MB*/
#define DEF_STACK_SIZE 8*1000*1024

/* Global scheduler variable */
static scheduler sched;

/* Counter for getting thread IDs */
static unsigned long thread_counter = 0;

/* Static/Private Functions */

/* Wrapper around lwpfuns */
static void lwp_wrap(lwpfun fun, void *arg);

/* Create a context */
static thread create_context();

/* Get stack size */
static size_t get_stack_size();

/* ---- */

tid_t lwp_create(lwpfun fun, void *arg)
{
    thread context_ptr;
    
    context_ptr = create_context();
    if (context_ptr == NULL)
    {
        return NO_THREAD;
    }




}

void lwp_wrap(lwpfun fun, void *arg)
{
    /* Call lwp_exit with fun's return value as its arg */
    lwp_exit(fun(arg));
}

thread create_context()
{
    thread context_ptr;
    
    context_ptr = (thread)malloc(sizeof(context));
    if (context_ptr == NULL)
    {
        return NULL;
    }

    thread_counter++;
    context_ptr->tid = (tid_t)thread_counter;

    context_ptr->stacksize = get_stack_size();
    if (context_ptr->stacksize == 0)
    {
        free(context_ptr);
        return NULL;
    }

    context_ptr->stack = mmap(NULL, context_ptr->stacksize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (context_ptr->stack == MAP_FAILED)
    {
        free(context_ptr);
        return NULL;
    }

    return context_ptr;
}

size_t get_stack_size()
{
    unsigned long page_size;
    unsigned long stack_limit;
    struct rlimit limit_info;

    page_size = sysconf(_SC_PAGE_SIZE);
    if (page_size == -1)
    {
        return 0;
    }

    if (getrlimit(RLIMIT_STACK, &limit_info) == -1 || limit_info.rlim_cur == RLIM_INFINITY)
    {
        stack_limit = DEF_STACK_SIZE;
    }
    else
    {
        stack_limit = limit_info.rlim_cur;
        stack_limit = ((stack_limit + page_size - 1) / page_size) * page_size;  /* Round up page to nearest page size */
    }

    return stack_limit;
}