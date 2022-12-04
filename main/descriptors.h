#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "adapter_includes.h"

// NS mode definitions
#define NS_HID_LEN      8
extern uint8_t          ns_buffer[NS_HID_LEN];

// Generic HID definitions
#define GP_HID_LEN      10
extern uint8_t          hid_buffer[GP_HID_LEN];

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
  uint8_t analog_trigger_l;
  uint8_t analog_trigger_r;
  uint8_t dummy_2;

} ns_input_s;

extern ns_input_s ns_input;
//Reuse ns_input_s for generic input
extern ns_input_s hid_input;

// GC mode definitions
/********************************/
#define GC_HID_LEN 37

// Input structure for Nintendo GameCube Adapter USB Data
typedef struct
{
    union
    {
        struct
        {
            uint8_t button_a    : 1;
            uint8_t button_b    : 1;
            uint8_t button_x    : 1;
            uint8_t button_y    : 1;
            uint8_t dpad_left   : 1; //Left
            uint8_t dpad_right  : 1; //Right
            uint8_t dpad_down   : 1; //Down
            uint8_t dpad_up     : 1; //Up
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t button_start: 1;
            uint8_t button_z    : 1;
            uint8_t button_r    : 1;
            uint8_t button_l    : 1;
            uint8_t blank1      : 4;
        }; 
        uint8_t buttons_2;
    };

  uint8_t stick_x;
  uint8_t stick_y;
  uint8_t cstick_x;
  uint8_t cstick_y;
  uint8_t trigger_l;
  uint8_t trigger_r;

} gc_input_s;

extern gc_input_s       gc_input;
extern uint8_t          gc_buffer[GC_HID_LEN];

// xInput mode definitions
/********************************/
#define XI_HID_LEN 11

typedef struct
{
    uint8_t  report_id;

    union
    {
        struct
        {
            uint8_t button_a    : 1; // Button 0
            uint8_t button_b    : 1; // Button 1
            uint8_t button_x    : 1; // Button 2
            uint8_t button_y    : 1; // Button 3
            uint8_t bumper_l    : 1; // Verified Fixed
            uint8_t bumper_r    : 1; // Verified Fixed
            uint8_t trigger_l   : 1; // Verified Fixed
            uint8_t trigger_r   : 1;
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t button_right_stick    : 1;
            uint8_t button_start    : 1;
            uint8_t button_guide    : 1;
            uint8_t dpad_up    : 1;
            uint8_t dpad_down : 1;
            uint8_t dpad_left: 1;
            uint8_t dpad_right: 1;
            uint8_t button_back : 1;
        }; 
        uint8_t buttons_2;
    };

    uint8_t dpad_hat;

    uint8_t stick_left_x;
    uint8_t stick_left_y;
    uint8_t stick_right_x;
    uint8_t stick_right_y;

    uint8_t analog_trigger_l;
    uint8_t analog_trigger_r;

    uint8_t dummy_1;

} xi_input_s;

extern xi_input_s       xi_input;
extern uint8_t          xi_buffer[XI_HID_LEN];

#endif