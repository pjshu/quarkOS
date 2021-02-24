//
// Created by pjs on 2021/2/23.
//

#ifndef QUARKOS_SCHED_TIMER_H
#define QUARKOS_SCHED_TIMER_H

#include "types.h"
#include "kthread.h"

#define TIMER_COUNT 20
typedef struct timer {
    volatile uint64_t time;
    struct timer *volatile next, *prev;
    tcb_t *thread;
} timer_t;


bool ms_sleep(mseconds_t msc);

bool ms_sleep_until(uint64_t msc);

void timer_handle();
void thread_timer_init();
#endif //QUARKOS_SCHED_TIMER_H