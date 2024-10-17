#ifndef NEW_RR_H
#define NEW_RR_H

#include <lwp.h>

/**
 * NOTES
 *  - init() and shutdown() are not defined
 *  - sched_one = next element
 *  - tail's next element is head
 */
/* Add the passed context to the scheduler's scheduling pool */
void new_rr_admit(thread new);

/* Remove the passed context from the scheduler's scheduling pool */
void new_rr_remove(thread victim);

/* Return the next thread to be run of NULL if there isn't one */
thread new_rr_next();

/* Return the number of runnable threads */
int new_rr_qlen();

#endif