#ifndef ADAPTER_INCLUDES_H
#define ADAPTER_INCLUDES_H

// Generics
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

// ESP includes
#include "esp_log.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "esp_heap_caps.h"
#include "esp_pm.h"
#include "esp_attr.h"
#include "esp_private/gdma.h"

// USB
#include "tinyusb.h"
#include "class/hid/hid_device.h"

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// Project
#include "sdkconfig.h"
#include "gc.h"
#include "rmt_gc.h"
#include "descriptors.h"
#include "util_rgb.h"
#include "util_usb.h"

// Drivers
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/rmt_types_legacy.h"
#include "driver/spi_master.h"

// HAL
#include "hal/rmt_types.h"
#include "hal/rmt_hal.h"
#include "hal/dma_types.h"
#include "hal/clk_gate_ll.h"
#include "hal/rmt_ll.h"

// SOC
#include "soc/soc_caps.h"
#include "soc/rmt_struct.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_periph.h"

// ROM
#include "rom/gpio.h"

#endif