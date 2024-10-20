/*
 */

#include "lwp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void indentnum(uintptr_t num) {
    /* print the number num num times, indented by 5*num spaces
     * Not terribly interesting, but it is instructive.
     */
    int id;
    id = (int)num;
    printf("Greetings from Thread %d.  Yielding...\n", id);
    lwp_yield();
    printf("I (%d) am still alive.  Goodbye.\n", id);
    lwp_exit(0);
}

int main(int argc, char *argv[]) {
    long i;

    /* spawn a number of individual LWPs */
    for (i = 0; i < 5; i++)
        lwp_create((lwpfun)indentnum, (void *)i);

    lwp_start();

    for (i = 0; i < 5; i++)
        lwp_wait(NULL);

    printf("LWPs have ended.\n");
    return 0;
}
