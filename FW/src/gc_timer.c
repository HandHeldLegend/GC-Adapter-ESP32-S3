#include "gc_timer.h"

gc_timer_status_t gc_timer_status = GC_TIMER_IDLE;

gptimer_handle_t gc_timer = NULL;


void gc_timer_init(void)
{

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gc_timer));
    ESP_ERROR_CHECK(gptimer_enable(gc_timer));
}

void gc_timer_stop(void)
{
    gptimer_stop(gc_timer);
    gc_timer_status = GC_TIMER_IDLE;
}

void gc_timer_start(void)
{
    gptimer_start(gc_timer);
    gc_timer_status = GC_TIMER_STARTED;
}

void gc_timer_reset(void)
{
    ESP_ERROR_CHECK(gptimer_set_raw_count(gc_timer, 0));
}