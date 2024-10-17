/*
 */

#include <lwp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <lwp.h>
#include "new_rr.h"

#define INITIALSTACK 2048
#define ROUNDS 6

static void indentnum(uintptr_t num) {
  /* print the number num num times, indented by 5*num spaces
   * Not terribly interesting, but it is instructive.
   */

  struct scheduler new_sched = {.init = NULL, .shutdown = NULL, .admit = new_rr_admit, .remove = new_rr_remove, .next = new_rr_next, .qlen = new_rr_qlen};
  int howfar,i;

  howfar=(int)num;              /* interpret num as an integer */
  for(i=0;i<ROUNDS;i++){
    printf("%*d\n",howfar*5,howfar);
    if ( num == 5 && i == 2 ) { /* end of third round */
      printf("Setting the scheduler.\n");
      lwp_set_scheduler(&new_sched);
    }

    lwp_yield();                /* let another have a turn */
  }
  lwp_exit(0);                   /* bail when done.  This should
                                 * be unnecessary if the stack has
                                 * been properly prepared
                                 */
}

int main(int argc, char *argv[]) {
    long i;

    /* spawn a number of individual LWPs */
    for (i = 1; i <= 5; i++)
        lwp_create((lwpfun)indentnum, (void *)i);

    
    printf("starting that jon");
    lwp_start();

    for (i = 1; i <= 5; i++)
        lwp_wait(NULL);

    printf("LWPs have ended.\n");
    return 0;
}
