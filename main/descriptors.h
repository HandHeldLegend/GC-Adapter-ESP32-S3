#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "adapter_includes.h"

// Dinput mode definitions
#define DI_HID_LEN  10
extern uint8_t          di_buffer[DI_HID_LEN];

// Input structure for DInput USB gamepad Data
typedef struct
{
    uint8_t report_id;

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
} __attribute__ ((packed)) di_input_s;

extern di_input_s di_input;

// NS mode definitions
#define NS_HID_LEN      8
extern uint8_t          ns_buffer[NS_HID_LEN];

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

} __attribute__ ((packed)) ns_input_s;

extern ns_input_s ns_input;

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

} __attribute__ ((packed)) gc_input_s;

extern gc_input_s       gc_input;
extern uint8_t          gc_buffer[GC_HID_LEN];

// xInput mode definitions
/********************************/
#define XI_HID_LEN 16

typedef struct
{
    uint8_t report_id : 8;

    uint16_t stick_left_x   : 16;
    uint16_t stick_left_y   : 16;
    uint16_t stick_right_x  : 16;
    uint16_t stick_right_y  : 16;

    uint16_t analog_trigger_l : 16;
    uint16_t analog_trigger_r : 16;

    uint8_t dpad_hat        : 4;
    uint8_t dpad_padding    : 4;

    union
    {
        struct
        {
            uint8_t button_a     : 1; 
            uint8_t button_b   : 1; 
            uint8_t button_x   : 1; 
            uint8_t button_y  : 1; 
            uint8_t bumper_l : 1; 
            uint8_t bumper_r : 1; 
            uint8_t button_back   : 1;  
            uint8_t button_menu  : 1;
        };
        uint8_t buttons_1 : 8;
    };

    union
    {
        struct
        {
            uint8_t button_stick_l        : 1;    
            uint8_t button_stick_r        : 1;    
            uint8_t padding    : 6;   
        }; 
        uint8_t buttons_2 : 8;
    };

} __attribute__ ((packed)) xi_input_s;

typedef struct
{
    uint8_t     report_id;
    uint16_t    padding_1;
    uint8_t     magnitude_l;
    uint8_t     magnitude_r;
    uint8_t     duration;
    uint8_t     start_delay;
    uint8_t     loop_count;
} __attribute__ ((packed)) xi_rumble_s;

extern xi_input_s       xi_input;
extern uint8_t          xi_buffer[XI_HID_LEN];

#define XID_REPORT_LEN 20

typedef struct
{
    uint8_t report_id;
	uint8_t report_size;
	union
    {
        struct
        {
            uint8_t dpad_up     : 1;
            uint8_t dpad_down   : 1; 
            uint8_t dpad_left      : 1;  
            uint8_t dpad_right      : 1;
            uint8_t button_menu     : 1; 
            uint8_t button_back     : 1; 
            uint8_t button_stick_l  : 1;
            uint8_t button_stick_r  : 1;
        };
        uint8_t buttons_1 : 8;
    };
	union
    {
        struct
        {
            uint8_t bumper_l    : 1;
            uint8_t bumper_r    : 1;
            uint8_t button_guide: 1; 
            uint8_t blank_1     : 1; 
            uint8_t button_a    : 1;
            uint8_t button_b    : 1; 
            uint8_t button_x    : 1; 
            uint8_t button_y    : 1; 
        };
        uint8_t buttons_2 : 8;
    };
	uint8_t analog_trigger_l;
	uint8_t analog_trigger_r;
	short stick_left_x;
	short stick_left_y;
	short stick_right_x;
	short stick_right_y;
	uint8_t reserved_1[6];
} __attribute__ ((packed)) xid_input_s;

extern xid_input_s xid_input;
extern uint8_t xid_buffer[XID_REPORT_LEN];

#endif