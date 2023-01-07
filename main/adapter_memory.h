#ifndef ADAPTER_MEMORY_H
#define ADAPTER_MEMORY_H

#include "adapter_includes.h"

// Corresponds to version number in web app.
#define MAGIC_NUM 0x08A6
#define SETTINGS_BYTE_LEN 8
#define SETTINGS_NAMESPACE "adapt_settings"

typedef struct
{
    uint16_t    magic_num;
    uint8_t     adapter_mode;
    uint8_t     led_brightness;

    union
    {
        struct
        {
            uint8_t     ns_trigger_l    : 2;
            uint8_t     ns_trigger_r    : 2;
            uint8_t     gc_trigger_l    : 2;
            uint8_t     gc_trigger_r    : 2;
            uint8_t     di_trigger_l    : 2;
            uint8_t     di_trigger_r    : 2;
            uint8_t     xi_trigger_l    : 2;
            uint8_t     xi_trigger_r    : 2;
        };
        uint16_t trigger_mode;
    };

    uint8_t     trigger_threshold_l;
    uint8_t     trigger_threshold_r;
} __attribute__ ((packed)) adapter_settings_s;

extern adapter_settings_s adapter_settings;

void load_adapter_mode(void);
void save_adapter_mode(void);
void load_adapter_defaults(void);

#endif