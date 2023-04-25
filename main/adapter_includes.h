#ifndef ADAPTER_INCLUDES_H
#define ADAPTER_INCLUDES_H

#define ADAPTER_DEBUG_ENABLE 0
#define NEXT_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
#define PREV_BUTTON (GPIO_NUM_4)


#define LED_BUTTON_PIN (GPIO_NUM_9)
#define LED_USB_PIN (GPIO_NUM_10)

#define LED_PIN_MASK ( (1ULL << LED_BUTTON_PIN) | (1ULL << LED_USB_PIN) ) 

#define PIN_MASK_GCP ( (1ULL << NEXT_BUTTON) | (1ULL << PREV_BUTTON))

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// Generics
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

// ESP includes
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "esp_heap_caps.h"
#include "esp_pm.h"
#include "esp_attr.h"
#include "esp_private/gdma.h"
#include "esp_check.h"
#include "rom/ets_sys.h"
#include "esp_rom_sys.h"
#include "driver/gptimer.h"
#include "rom/ets_sys.h"

// USB
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "class/vendor/vendor_device.h"
#include "tusb_tasks.h"
#include "device/usbd_pvt.h"
#include "xinput_device.h"

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// Project
#include "sdkconfig.h"
#include "adapter_types.h"
#include "gc.h"
#include "rmt_gc.h"
#include "descriptors.h"
#include "util_usb.h"
#include "gc_timer.h"
#include "adapter_memory.h"
#include "command_handler.h"
#include "util_common.h"

// Drivers
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/rmt_types_legacy.h"
#include "esp32_neopixel.h"
#include "rgb_animator.h"

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

// Storage
#include "nvs_flash.h"
#include "nvs.h"

#endif