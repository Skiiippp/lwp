#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include <lwp.h>

/**
 * NOTES
 *  - init() and shutdown() are not defined
 *  - sched_one = next element
 *  - tail's next element is head
 */
/* Add the passed context to the scheduler's scheduling pool */
void rr_admit(thread new);

/* Remove the passed context from the scheduler's scheduling pool */
void rr_remove(thread victim);

/* Return the next thread to be run of NULL if there isn't one */
thread rr_next();

/* Return the number of runnable threads */
int rr_qlen();

#endif