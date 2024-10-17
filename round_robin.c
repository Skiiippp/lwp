#include "round_robin.h"

#include <assert.h>
#include <stddef.h>

static thread rr_head = NULL, rr_tail = NULL;
static size_t queue_length = 0;

/* -- Static/Private Functions -- */

/* Helper function for removing thread */
static void remove_thread(thread prev_thread, thread this_thread);

/* ---- */

void rr_admit(thread new) {
    thread old_tail;

    if (queue_length == 0) {
        rr_head = new;
        rr_tail = new;
    } else {
        old_tail = rr_tail;
        rr_tail = new;
        old_tail->sched_one = rr_tail;
    }

    rr_tail->sched_one = rr_head;
    queue_length++;
}

void rr_remove(thread victim) {
    thread this_thread = rr_head, prev_thread = rr_tail;

    assert(queue_length > 0);

    for (int i = 0; i < queue_length; i++) {
        if (this_thread->tid == victim->tid) {
            remove_thread(prev_thread, this_thread);
            return;
        }
        prev_thread = this_thread;
        this_thread = this_thread->sched_one;
    }

    assert(FALSE);
}

thread rr_next() {
    thread next_thread = rr_head;
    thread next_prev_thread = rr_tail;

    if (next_thread == NULL) {
        return NULL;
    }

    /* Remove next thread. Its prev is the tail */
    remove_thread(next_prev_thread, next_thread);

    return next_thread;
}

int rr_qlen() { return queue_length; }

void remove_thread(thread prev_thread, thread this_thread) {
    if (queue_length == 1) {
        rr_head = NULL;
        rr_tail = NULL;
    } else {
        prev_thread->sched_one = this_thread->sched_one;

        if (this_thread == rr_head) {
            rr_head = rr_head->sched_one;
        } else if (this_thread == rr_tail) {
            rr_tail = prev_thread;
        }
    }

    queue_length--;
}