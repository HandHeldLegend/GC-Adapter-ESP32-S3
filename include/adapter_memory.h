#ifndef ADAPTER_MEMORY_H
#define ADAPTER_MEMORY_H

#include "adapter_includes.h"

// Corresponds to version number in web app.
#define FIRMWARE_VERSION 0x0901
#define SETTINGS_VERSION 0x00F1
#define SETTINGS_BYTE_LEN 8
#define SETTINGS_NAMESPACE "adapt_settings"

typedef struct
{
    // We use a settings version to
    // keep settings between updates
    uint16_t    settings_version;
    uint8_t     switch_mac_address[6];
    uint8_t     adapter_mode;
    uint8_t     led_brightness;

    union
    {
        struct
        {
            uint8_t     trigger_mode_l    : 4;
            uint8_t     trigger_mode_r    : 4;
        };
        uint8_t trigger_mode;
    };

    uint8_t     trigger_threshold_l;
    uint8_t     trigger_threshold_r;

    uint8_t     zjump;

    bool performance_mode;

} __attribute__ ((packed)) adapter_settings_s;

extern adapter_settings_s adapter_settings;

void load_adapter_settings(void);
void save_adapter_settings(void);
void load_adapter_defaults(void);

#endif
