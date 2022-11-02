#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "adapter_includes.h"

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

// NS mode definitions
#define NS_HID_LEN  8
extern const uint8_t ns_hid_report_descriptor[94];
extern uint8_t ns_buffer[NS_HID_LEN];
extern const uint8_t ns_hid_configuration_descriptor[];

// Input structure for Nintendo Switch USB gamepad Data
typedef struct
{
    union
    {
        struct
        {
            uint8_t button_y    : 1;
            uint8_t button_b    : 1;
            uint8_t button_a    : 1;
            uint8_t button_x    : 1;
            uint8_t trigger_l   : 1;
            uint8_t trigger_r   : 1;
            uint8_t trigger_zl  : 1;
            uint8_t trigger_zr  : 1;
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t button_minus  : 1;
            uint8_t button_plus   : 1;
            uint8_t stick_left    : 1;
            uint8_t stick_right   : 1;
            uint8_t button_home   : 1;
            uint8_t button_capture: 1;
            uint8_t dummy_1       : 2;
        }; 
        uint8_t buttons_2;
    };

  uint8_t dpad_hat;
  uint8_t stick_left_x;
  uint8_t stick_left_y;
  uint8_t stick_right_x;
  uint8_t stick_right_y;
  uint8_t dummy_2;

} ns_input_s;

extern ns_input_s ns_input;

// GC mode definitions
#define GC_HID_LEN 37
extern const uint8_t gc_hid_report_descriptor[];
extern uint8_t gc_buffer[GC_HID_LEN];

// Generic mode definitions
// Generic mode is used for configuration as well.
/*
A B X Y L R START SELECT
Z U1 U2 U3 U4 U5 U6 U7
DPAD HAT
LX
LY
RX
RY
TLA
TRA
*/
#define GENERIC_HID_LEN 9
extern const uint8_t generic_hid_report_descriptor[];

#endif