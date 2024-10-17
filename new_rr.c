#include "new_rr.h"

#include <assert.h>
#include <stddef.h>

static thread new_rr_head = NULL, new_rr_tail = NULL;
static size_t queue_length = 0;

/* -- Static/Private Functions -- */

/* Helper function for removing thread */
static void remove_thread(thread prev_thread, thread this_thread);

/* ---- */


void new_rr_admit(thread new) {
    thread old_tail;

    if (queue_length == 0) {
        new_rr_head = new;
        new_rr_tail = new;
    } else {
        old_tail = new_rr_tail;
        new_rr_tail = new;
        old_tail->sched_one = new_rr_tail;
    }

    new_rr_tail->sched_one = new_rr_head;
    queue_length++;
}

void new_rr_remove(thread victim) {
    thread this_thread = new_rr_head, prev_thread = new_rr_tail;

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

thread new_rr_next() {
    thread next_thread = new_rr_head;

    if (next_thread == NULL) {
        return NULL;
    }

    /* Make new new_rr_tail = new_rr_head, new new_rr_head = old new_rr_head's next */
    new_rr_tail->sched_one = new_rr_head;
    new_rr_head = new_rr_head->sched_one;
    new_rr_tail = next_thread;

    return next_thread;
}

int new_rr_qlen() { return queue_length; }

void remove_thread(thread prev_thread, thread this_thread) {
    if (queue_length == 1) {
        new_rr_head = NULL;
        new_rr_tail = NULL;
    } else {
        prev_thread->sched_one = this_thread->sched_one;

        if (this_thread == new_rr_head) {
            new_rr_head = new_rr_head->sched_one;
        } else if (this_thread == new_rr_tail) {
            new_rr_tail = prev_thread;
        }
    }

    queue_length--;
}