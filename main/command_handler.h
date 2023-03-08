#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "adapter_includes.h"

#define CMD_USB_REPORTID    0x02
#define CMD_USB_REPORTLEN   10

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

typedef enum
{
    CMD_SETTINGS_DEFAULT    = 0x00,
    CMD_SETTINGS_SAVEALL    = 0x01,
    CMD_SETTINGS_GETALL     = 0x02,
    CMD_SETTINGS_LEDBRIGHTNESS  = 0x03,
    CMD_SETTINGS_TRIGGERMODE    = 0x04,
    CMD_SETTINGS_TRIGGERSENSITIVITY = 0x05,
    CMD_SETTINGS_ZJUMP              = 0x06,
    CMD_SETTINGS_SETTINGVERSION     = 0x07,
    CMD_SETTINGS_FWVERSION          = 0x08,
} settings_cmd_t;

extern bool cmd_flagged;
extern uint8_t cmd_queue[8];
extern uint8_t cmd_queue_idx;
extern uint8_t cmd_buffer[CMD_USB_REPORTLEN];

void command_queue_process(void);

void command_handler(const uint8_t *data, uint16_t bufsize);

#endif