#ifndef RMT_GC_H
#define RMT_GC_H

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "soc/soc_caps.h"
#include "hal/rmt_types.h"
#include "hal/rmt_hal.h"
#include "hal/dma_types.h"
#include "esp_intr_alloc.h"
#include "esp_heap_caps.h"
#include "esp_pm.h"
#include "esp_attr.h"
#include "esp_private/gdma.h"
#include "driver/rmt_types_legacy.h"

typedef struct {
    struct {
        volatile rmt_item32_t data32[SOC_RMT_MEM_WORDS_PER_CHANNEL];
    } chan[SOC_RMT_CHANNELS_PER_GROUP];
} legacy_rmt_mem_t;

// RMTMEM address is declared in <target>.peripherals.ld
extern legacy_rmt_mem_t RMTMEM;


#endif
