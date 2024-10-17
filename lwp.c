#include <lwp.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include "round_robin.h"

/* REMOVE FOR SUBMISSION & TESTING - ONLY FOR LINTER*/
//#define MAP_ANONYMOUS 0
//#define MAP_STACK 0

/* If RLIMIT_STACK is unreasonable - 8MB*/
#define DEF_STACK_SIZE 8 * 1000 * 1024

/* Bytes per word in 64 bit system */
#define BYTES_PER_WORD 8

/* Global scheduler variable */
static scheduler sched = NULL;

/* Round robin scheduler */
static struct scheduler round_robin = {.init = NULL,
                                       .shutdown = NULL,
                                       .admit = rr_admit,
                                       .remove = rr_remove,
                                       .next = rr_next,
                                       .qlen = rr_qlen};
                                       

/* Counter for getting thread IDs */
static unsigned long thread_counter = 0;

/* Currently running thread */
static thread current_thread = NULL;

/* Head and tail for internal thread linked list for getting tids. Uses lib_one
 * to point to next*/
static thread head = NULL, tail = NULL;

/* Number of threads */
static size_t number_threads = 0;

/* Head and tail for internel waiting thread queue. Uses lib_two to point to
 * next */
static thread oldest_waiting = NULL, newest_waiting = NULL;

/* Tracking oldest (head) and newest (tail) exited threads. Uses exited to
 * point to next */
static thread oldest_exited = NULL, newest_exited = NULL;

/* -- Static/Private Functions -- */

/* Initialize the stack*/
static void init_stack(thread context_ptr);

/* Initialize the rfile */
static void init_rfile(thread context_ptr, lwpfun fun, void *arg);

/* Wrapper around lwpfuns */
static void lwp_wrap(lwpfun fun, void *arg);

/* Set the value of an address in the stack at the supplied word offset */
static void set_stack_value_at_offset(thread context_ptr, size_t offset,
                                      unsigned long value);

/* Get the address (as unsigned long) of some word offset from the top of the
 * stack */
static unsigned long get_stack_addr_at_offset(thread context_ptr,
                                              size_t offset);

/* Create a context (ie allocate context struct and stack)*/
static thread create_context();

/* Get stack size in bytes */
static size_t get_stack_size();

/* Add new thread to list of threads */
static void add_to_list(thread new);

/* Set a new thread's tid */
static void set_tid(thread new);

/* Deallocate a thread's context */
static void deallocate_context(thread dead_thread);

/* Add an exited thread to the exit queue */
static void add_to_exit_queue(thread exited);

/* Pop an exited thread from exit queue */
static thread pop_from_exiting_queue();

/* Add a waiting thread to the waiting thread queue */
static void add_to_waiting_queue(thread waiting);

/* Pop a waiting thread off the waitng thread queue */
static thread pop_from_waiting_queue();

/* ---- */

tid_t lwp_create(lwpfun fun, void *arg) {
    thread context_ptr;

    context_ptr = create_context();
    if (context_ptr == NULL) {
        return NO_THREAD;
    }

    context_ptr->status = LWP_LIVE;

    set_tid(context_ptr);

    init_stack(context_ptr);
    init_rfile(context_ptr, fun, arg);

    /* Check if we have a scheduler */
    if (sched == NULL) {
        lwp_set_scheduler(NULL);
    }

    sched->admit(context_ptr);

    add_to_list(context_ptr);

    return context_ptr->tid;
}

void lwp_exit(int status) {
    thread unblocked_thread;

    current_thread->status = MKTERMSTAT(LWP_TERM, status);
    sched->remove(current_thread);
    add_to_exit_queue(current_thread);

    ////printf("Exiting thread %i\n", current_thread->tid);

    /* Check if there are any blocked threads waiting for exits */
    if (oldest_waiting != NULL) {
        /* Pop and deallocate oldest exited thread, and place oldest waiting
         * back in the scheduler */
        unblocked_thread = pop_from_waiting_queue();
        // printf("Unblocking thread %i\n", unblocked_thread->tid);
        sched->admit(unblocked_thread);
    }

    lwp_yield();
}

tid_t lwp_gettid() {
    if (current_thread == NULL) {
        return NO_THREAD;
    }

    return current_thread->tid;
}

void lwp_yield() {
    thread old_thread;

    /*
    old_thread = current_thread;
    current_thread = sched->next();
    sched->remove(current_thread);
    sched->admit(old_thread);
    */
    old_thread = current_thread;
    current_thread = sched->next();

    // printf("Thread %i yeilding to %i\n", old_thread->tid,
    // current_thread->tid);

    if (current_thread == NULL) {
        exit(old_thread->status);
    }

    /*
    if (old_thread == NULL)
    {
        swap_rfiles(NULL, &current_thread->state);
    }*/

    swap_rfiles(&old_thread->state, &current_thread->state);
}

void lwp_start() {
    thread calling_thread;

    /* If user hasn't set scheduler, set it to round robin */
    if (sched == NULL) {
        lwp_set_scheduler(NULL);
    }

    calling_thread = (thread)malloc(sizeof(context));

    thread_counter++;
    calling_thread->tid = (tid_t)thread_counter;

    /* How we tell if a thread is the original thread */
    calling_thread->stack = NULL;

    sched->admit(calling_thread);

    current_thread = calling_thread;

    add_to_list(calling_thread);

    /* Get state */
    // swap_rfiles(&calling_thread->state, NULL);

    lwp_yield();
}

tid_t lwp_wait(int *status) {
    thread exited = NULL;
    tid_t exited_tid;

    /* Should return after either one or two loops */
    while (exited == NULL) {
        /* Check if there are terminated threads that haven't been cleaned up
         */
        if (oldest_exited != NULL) {
            exited = pop_from_exiting_queue();
            exited_tid = exited->tid;

            if (status != NULL) {
                *status = exited->status;
            }

            deallocate_context(exited);
            return exited_tid;
        } else if (sched->qlen() <= 1) {
            /* No threads that could block */
            return NO_THREAD;
        } else {
            /* Block until lwp_exit() calls us back */
            // printf("Blocking thread %i\n", current_thread->tid);
            add_to_waiting_queue(current_thread);
            sched->remove(current_thread);
            lwp_yield();
        }
    }

    /* Should never reach here */
    return (tid_t)(-1);
}

void lwp_set_scheduler(scheduler fun) {
    scheduler new_scheduler;
    thread temp_thread;
    size_t thread_transfer_counter;

    if (sched == NULL) {
        if (fun == NULL) {
            // fun = &round_robin;
            fun = &round_robin;
        }
        sched = fun;
        return;
    }

    if (fun == sched) {
        return;
    }

    if (fun == NULL) {
        // new_scheduler = &round_robin;
        new_scheduler = &round_robin;
    } else {
        new_scheduler = fun;
    }

    if (new_scheduler->init != NULL) {
        new_scheduler->init();
    }

    thread_transfer_counter = sched->qlen() - 1;
    new_scheduler->admit(current_thread);
    sched->remove(current_thread);
    while (thread_transfer_counter > 0) {
        temp_thread = sched->next();
        sched->remove(temp_thread);
        new_scheduler->admit(temp_thread);
        thread_transfer_counter--;
    }

    if (sched->shutdown != NULL) {
        sched->shutdown();
    }

    sched = new_scheduler;
}

scheduler lwp_get_scheduler() { return sched; }

void init_stack(thread context_ptr) {
    /* Make a function pointer to lwp_wrap */
    void (*wrapper)(lwpfun, void *) = lwp_wrap;

    /* Set -1 word offset address value to be function pointer to the wrapper
     */
    set_stack_value_at_offset(context_ptr, 1,
                              (unsigned long)(uintptr_t)wrapper);

    /* Set -2 word offset address value to be base ptr, will be 0 word offset
     * address */
    set_stack_value_at_offset(
        context_ptr, // size_t byte_offset = (offset + 1) * BYTES_PER_WORD;
        2, get_stack_addr_at_offset(context_ptr, 1));
}

static void init_rfile(thread context_ptr, lwpfun fun, void *arg) {
    /* Get current rfile as a template */
    swap_rfiles(&context_ptr->state, NULL);

    /* Set stack and base pointers to be at -2 word offset */
    context_ptr->state.rsp = get_stack_addr_at_offset(context_ptr, 2);
    context_ptr->state.rbp = context_ptr->state.rsp;

    /* Set first arg to be lwp_wrap to be fun, second to be arg */
    context_ptr->state.rdi = (unsigned long)(uintptr_t)fun;
    context_ptr->state.rsi = (unsigned long)(uintptr_t)arg;
}

void set_stack_value_at_offset(thread context_ptr, size_t offset,
                               unsigned long value) {
    context_ptr->stack[context_ptr->stacksize / BYTES_PER_WORD - offset - 1] =
        value;
}

unsigned long get_stack_addr_at_offset(thread context_ptr, size_t offset) {
    return (
        unsigned long)((uintptr_t)&context_ptr
                           ->stack[context_ptr->stacksize / BYTES_PER_WORD -
                                   offset - 1]);
}

void lwp_wrap(lwpfun fun, void *arg) {
    /* Call lwp_exit with fun's return value as its arg */
    lwp_exit(fun(arg));
}

thread create_context() {
    thread context_ptr;

    context_ptr = (thread)malloc(sizeof(context));
    if (context_ptr == NULL) {
        return NULL;
    }

    context_ptr->stacksize = get_stack_size();
    if (context_ptr->stacksize == 0) {
        free(context_ptr);
        return NULL;
    }

    context_ptr->stack = (unsigned long *)mmap(
        NULL, context_ptr->stacksize, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (context_ptr->stack == MAP_FAILED) {
        free(context_ptr);
        return NULL;
    }

    return context_ptr;
}

size_t get_stack_size() {
    unsigned long page_size;
    unsigned long stack_limit;
    struct rlimit limit_info;

    page_size = sysconf(_SC_PAGE_SIZE);
    if (page_size == -1) {
        return 0;
    }

    if (getrlimit(RLIMIT_STACK, &limit_info) == -1 ||
        limit_info.rlim_cur == RLIM_INFINITY) {
        stack_limit = DEF_STACK_SIZE;
    } else {
        stack_limit = limit_info.rlim_cur;
        stack_limit = ((stack_limit + page_size - 1) / page_size) *
                      page_size; /* Round up page to nearest page size */
    }

    return stack_limit;
}

void add_to_list(thread new) {
    thread old_tail;

    if (number_threads == 0) {
        head = new;
        tail = new;
    } else {
        /* lib_one points to next thread in linked list */
        old_tail = tail;
        tail = new;
        old_tail->lib_one = tail;
    }

    tail->lib_one = head;
    number_threads++;
}

void set_tid(thread new) {
    thread_counter++;
    new->tid = (tid_t)thread_counter;
}

void deallocate_context(thread dead_thread) {

    /* Deallocate stack if not original thread */
    if (dead_thread->stack != NULL &&
        munmap((void *)dead_thread->stack, (size_t)dead_thread->stacksize) !=
            0) {
        perror("Failed to dealloc stack");
        return;
    }

    /* Free context struct */
    free(dead_thread);
}

void add_to_exit_queue(thread exited) {
    thread old_exit_tail;

    /* Queue empty */
    if (oldest_exited == NULL) {
        oldest_exited = exited;
        newest_exited = exited;
    } else {
        old_exit_tail = newest_exited;
        newest_exited = exited;
        old_exit_tail->exited = newest_exited;
    }

    newest_exited->exited = oldest_exited; /* Loop around */
}

thread pop_from_exiting_queue() {
    thread popped_thread;

    assert(oldest_exited != NULL);

    popped_thread = oldest_exited;

    if (oldest_exited == newest_exited) {
        oldest_exited = NULL;
        newest_exited = NULL;
    } else {
        oldest_exited = oldest_exited->lib_two;
    }

    return popped_thread;
}

void add_to_waiting_queue(thread waiting) {
    thread old_waiting_tail;

    if (oldest_waiting == NULL) {
        oldest_waiting = waiting;
        newest_waiting = waiting;
    } else {
        old_waiting_tail = newest_waiting;
        newest_waiting = waiting;
        old_waiting_tail->lib_two = newest_waiting;
    }

    newest_waiting->lib_two = oldest_waiting;
}

thread pop_from_waiting_queue() {
    thread popped_thread;

    /* Should never be called if queue is empty */
    assert(oldest_waiting != NULL);

    popped_thread = oldest_waiting;

    /* Check if size of one */
    if (oldest_waiting == newest_waiting) {
        oldest_waiting = NULL;
        newest_waiting = NULL;
    } else {
        oldest_waiting = oldest_waiting->lib_two;
    }

    return popped_thread;
}