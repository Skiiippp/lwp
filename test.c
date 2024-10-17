
#include "lwp.h"
#include "new_rr.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ROUNDS 6

typedef void (*sigfun)(int signum);
static void indentnum(uintptr_t num);

struct scheduler AltRoundRobin = {.init = NULL,
                                  .shutdown = NULL,
                                  .admit = new_rr_admit,
                                  .next = new_rr_next,
                                  .remove = new_rr_remove,
                                  .qlen = new_rr_qlen};

int main(int argc, char *argv[]) {
    long i;

    printf("Setting Scheduler\n");
    lwp_set_scheduler(&AltRoundRobin);

    printf("Creating LWPS\n");
    /* spawn a number of individual LWPs */
    for (i = 1; i <= 5; i++) {
        lwp_create((lwpfun)indentnum, (void *)i);
    }

    printf("Launching LWPS\n");
    lwp_start(); /* returns when the last lwp exits */

    for (i = 1; i <= 5; i++) {
        lwp_wait(NULL);
    }
    printf("Back from LWPS.\n");
    return 0;
}

static void indentnum(uintptr_t num) {
    /* print the number num num times, indented by 5*num spaces
     * Not terribly interesting, but it is instructive.
     */
    int howfar, i;

    howfar = (int)num; /* interpret num as an integer */
    for (i = 0; i < ROUNDS; i++) {
        printf("%*d\n", howfar * 5, howfar);
        if (num == 5 && i == 2) { /* end of third round */
            printf("Setting the scheduler to itself.\n");
            lwp_set_scheduler(&AltRoundRobin);
        }

        lwp_yield(); /* let another have a turn */
    }
    lwp_exit(0); /* bail when done.  This should
                  * be unnecessary if the stack has
                  * been properly prepared
                  */
}
