#include <lwp.h>
#include <stdio.h>

/* Global scheduler variable */
static scheduler sched;

/* Static/Private Functions */

static void lwp_wrap(lwpfun fun, void *arg);

/* ---- */

tid_t lwp_create(lwpfun fun, void *arg)
{
    
}

void lwp_wrap(lwpfun fun, void *arg)
{
    /* Call lwp_exit with fun's return value as its arg */
    lwp_exit(fun(arg));
}
