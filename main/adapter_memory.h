#ifndef ADAPTER_MEMORY_H
#define ADAPTER_MEMORY_H

#include "adapter_includes.h"

#define MAGIC_NUM 0xFF83
#define SETTINGS_BYTE_LEN 8
#define SETTINGS_NAMESPACE "adapt_settings"

typedef struct
{
    uint16_t    magic_num;
    uint8_t     adapter_mode;
    uint8_t     led_brightness;
    uint16_t    trigger_mode;
    uint8_t     trigger_threshold_l;
    uint8_t     trigger_threshold_r;
} __attribute__ ((packed)) adapter_settings_s;

extern adapter_settings_s adapter_settings;

void load_adapter_mode(void);
void save_adapter_mode(void);
void load_adapter_defaults(void);

#endif