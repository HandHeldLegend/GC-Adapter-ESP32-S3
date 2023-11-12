#ifndef GC_TIMER_H
#define GC_TIMER_H

#include "adapter_includes.h"

typedef enum
{
    GC_TIMER_IDLE,
    GC_TIMER_STARTED,
} gc_timer_status_t;

extern gptimer_handle_t gc_timer;
extern gc_timer_status_t gc_timer_status;

void gc_timer_init(void);
void gc_timer_stop(void);
void gc_timer_start(void);
void gc_timer_reset(void);

#endif