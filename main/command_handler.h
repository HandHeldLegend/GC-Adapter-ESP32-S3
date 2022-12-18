#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "adapter_includes.h"

#define CMD_SET_DEF_LEN 2
#define CMD_SET_SAVEALL_LEN 2
#define CMD_SET_BULKALL_LEN 10
#define CMD_SET_GETALL_LEN  2
#define CMD_SET_LEDBR_LEN   3
#define CMD_SET_TRIGMOD_LEN 4
#define CMD_SET_TRIGSEN_LEN 4

#define CMD_USB_REPORTID    0x02
#define CMD_USB_REPORTLEN   0x0A

typedef enum
{
    TRIG_MODE_OFF,
    TRIG_MODE_A2D,
    TRIG_MODE_D2ALITE,
    TRIG_MODE_D2AFULL,
} trigger_mode_t;

// Trigger modes for each individual controller mode.
// 0x0 - Analog does nothing or acts as passthrough. Digital button works as normal.
// 0x1 - Analog threshold applies digital press.
// 0x2 - Digital button activates analog half press. Good for trigger plugs.
// 0x3 - Digital button activates analog full press. Good for trigger plugs. 
typedef struct
{
    uint8_t     ns_trigger_l    : 2;
    uint8_t     ns_trigger_r    : 2;
    uint8_t     gc_trigger_l    : 2;
    uint8_t     gc_trigger_r    : 2;
    uint8_t     di_trigger_l    : 2;
    uint8_t     di_trigger_r    : 2;
    uint8_t     xi_trigger_l    : 2;
    uint8_t     xi_trigger_r    : 2;
} __attribute__ ((packed)) trigger_mode_s;

extern trigger_mode_s trigger_mode;

typedef enum
{
    CMD_SETTINGS_DEFAULT    = 0x00,
    CMD_SETTINGS_SAVEALL    = 0x01,
    CMD_SETTINGS_GETALL     = 0x02,
    CMD_SETTINGS_LEDBRIGHTNESS  = 0x03,
    CMD_SETTINGS_TRIGGERMODE    = 0x04,
    CMD_SETTINGS_TRIGGERSENSITIVITY = 0x05,
    CMD_SETTINGS_BULKALL            = 0x06,
} settings_cmd_t;

void command_handler(const uint8_t *data, uint16_t bufsize);

#endif