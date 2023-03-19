#ifndef ADAPTER_MEMORY_H
#define ADAPTER_MEMORY_H

#include "adapter_includes.h"

// Corresponds to version number in web app.
#define FIRMWARE_VERSION 0x08AD
#define SETTINGS_VERSION 0x08A7
#define SETTINGS_BYTE_LEN 8
#define SETTINGS_NAMESPACE "adapt_settings"

typedef struct
{
    // We use a settings version to
    // keep settings between updates
    uint16_t    settings_version;
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

    union
    {
        struct
        {
            uint8_t ns_zjump : 2;
            uint8_t gc_zjump : 2;
            uint8_t di_zjump : 2;
            uint8_t xi_zjump : 2;
        };
        uint8_t zjump;
    };

} __attribute__ ((packed)) adapter_settings_s;

extern adapter_settings_s adapter_settings;

void load_adapter_settings(void);
void save_adapter_settings(void);
void load_adapter_defaults(void);

#endif