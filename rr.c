#include "rr.h"
#include <lwp.h>
#include <stdio.h>
#include <stdlib.h>

static thread qhead = NULL;
#define tnext sched_one
#define tprev sched_two
int num = 0;

static void rr_admit(thread new) {

    /* add to queue */
    if (qhead) {
        new->tnext = qhead;
        new->tprev = qhead->tprev;
        new->tprev->tnext = new;
        qhead->tprev = new;
    } else {
        qhead = new;
        qhead->tnext = new;
        qhead->tprev = new;
    }
    num++;
}

static void rr_remove(thread victim) {
    /* cut out of queue */
    victim->tprev->tnext = victim->tnext;
    victim->tnext->tprev = victim->tprev;

    /* what if it were qhead? */
    if (victim == qhead) {
        if (victim->tnext != victim)
            qhead = victim->tprev; /* preserve who would've been next */
        else
            qhead = NULL;
    }
    num--;
}

static thread rr_next() {
    static int firsttime = TRUE;

    if (firsttime)
        firsttime = FALSE;
    else if (qhead)
        qhead = qhead->tnext;

    return qhead;
}

static int rr_qlen() { return num; }

static struct scheduler rr_publish = {NULL,      NULL,    rr_admit,
                                      rr_remove, rr_next, rr_qlen};
scheduler AltRoundRobin = &rr_publish;

/*********************************************************/
__attribute__((unused)) void dpl() {
    thread l;
    if (!qhead)
        fprintf(stderr, "qhead is NULL\n");
    else {
        fprintf(stderr, "queue:\n");
        l = qhead;
        do {
            fprintf(stderr, "  (tid=%lu tnext=%p tprev=%p)\n", l->tid,
                    l->tnext, l->tprev);
            l = l->tnext;
        } while (l != qhead);
        fprintf(stderr, "\n");
    }
}
