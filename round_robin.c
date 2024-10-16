#include "round_robin.h"

#include <assert.h>
#include <stddef.h>

static thread head = NULL, tail = NULL;
static size_t queue_length = 0;

/* -- Static/Private Functions -- */

/* Helper function for removing thread */
static void remove_thread(thread prev_thread, thread this_thread);

/* ---- */

void rr_admit(thread new) {
    if (queue_length == 0) {
        head = new;
    } else {
        tail->sched_one = new;
    }

    tail = new;
    tail->sched_one = head;
    queue_length++;
}

void rr_remove(thread victim) {
    thread this_thread = head, prev_thread = tail;

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
    thread next_thread = head;

    if (next_thread == NULL) {
        return NULL;
    }

    /* Make new tail = head, new head = old head's next */
    tail->sched_one = head;
    head = head->sched_one;
    tail = next_thread;

    return next_thread;
}

int rr_qlen() { return queue_length; }

void remove_thread(thread prev_thread, thread this_thread) {
    if (queue_length == 1) {
        head = NULL;
        tail = NULL;
    } else {
        prev_thread->sched_one = this_thread->sched_one;
    }

    queue_length--;
}