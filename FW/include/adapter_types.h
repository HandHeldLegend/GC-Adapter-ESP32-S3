#ifndef ADAPTER_TYPES_H
#define ADAPTER_TYPES_H

typedef enum
{
    CMD_PHASE_PROBE,
    CMD_PHASE_ORIGIN,
    CMD_PHASE_POLL,
} gc_cmd_phase_t;

typedef enum
{
    GC_USB_IDLE,
    GC_USB_OK,
} gc_usb_phase_t;

typedef enum
{
    GC_TYPE_UNKNOWN     = 0x00,
    GC_TYPE_WIRED       = 0x09,
    GC_TYPE_WAVEBIRD    = 0xE9,
    GC_TYPE_KEYBOARD,
} gc_type_t;

typedef enum
{
    USB_MODE_NS       = 0x00,
    USB_MODE_GC       = 0x01,
    USB_MODE_XINPUT   = 0x02,
    USB_MODE_MAX,
} usb_mode_t;

typedef enum
{
    TRIG_MODE_OFF,
    TRIG_MODE_A2D,
    TRIG_MODE_D2ALITE,
    TRIG_MODE_D2AFULL,
} trigger_mode_t;

typedef struct
{
    uint8_t id_upper;
    uint8_t id_lower;
    uint8_t junk;
} __attribute__ ((packed)) gc_probe_response_s;

typedef struct
{
    union 
    {
        struct
        {
            uint8_t button_a        : 1;
            uint8_t button_b        : 1;
            uint8_t button_x        : 1;
            uint8_t button_y        : 1;
            uint8_t button_start    : 1;
            uint8_t wavebird_connected : 1;
            uint8_t b1blank         : 2;  
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t dpad_left       : 1;
            uint8_t dpad_right      : 1;
            uint8_t dpad_down       : 1;
            uint8_t dpad_up         : 1;
            uint8_t button_z        : 1;
            uint8_t button_r        : 1;
            uint8_t button_l        : 1;
            uint8_t b2blank         : 1; 
        };
        uint8_t buttons_2;
    };

    uint8_t stick_x;
    uint8_t stick_y;
    uint8_t cstick_x;
    uint8_t cstick_y;
    uint8_t trigger_l;
    uint8_t trigger_r;
} __attribute__ ((packed)) gc_poll_response_s;

extern gc_poll_response_s   gc_poll_response;

typedef struct
{
    bool    data_set;
    int     stick_x;
    int     stick_y;
    int     cstick_x;
    int     cstick_y;
    int     trigger_l;
    int     trigger_r;
} __attribute__ ((packed)) gc_origin_data_s;

// XInput mode definitions
/********************************/
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

// Nintendo Switch Input Type
typedef struct
{
    union
    {
        struct
        {
            // Y and C-Up (N64)
            uint8_t b_y       : 1;


            // X and C-Left (N64)
            uint8_t b_x       : 1;

            uint8_t b_b       : 1;
            uint8_t b_a       : 1;
            uint8_t t_r_sr    : 1;
            uint8_t t_r_sl    : 1;
            uint8_t t_r       : 1;

            // ZR and C-Down (N64)
            uint8_t t_zr      : 1;
        };
        uint8_t right_buttons;
    };
    union
    {
        struct
        {
            // Minus and C-Right (N64)
            uint8_t b_minus     : 1;

            // Plus and Start
            uint8_t b_plus      : 1;

            uint8_t sb_right    : 1;
            uint8_t sb_left     : 1;
            uint8_t b_home      : 1;
            uint8_t b_capture   : 1;
            uint8_t none        : 1;
            uint8_t charge_grip_active : 1;
        };
        uint8_t shared_buttons;
    };
    union
    {
        struct
        {
            uint8_t d_down    : 1;
            uint8_t d_up      : 1;
            uint8_t d_right   : 1;
            uint8_t d_left    : 1;
            uint8_t t_l_sr    : 1;
            uint8_t t_l_sl    : 1;
            uint8_t t_l       : 1;

            // ZL and Z (N64)
            uint8_t t_zl      : 1;

        };
        uint8_t left_buttons;
    };

    uint16_t ls_x;
    uint16_t ls_y;
    uint16_t rs_x;
    uint16_t rs_y;

} __attribute__ ((packed)) sw_input_s;

#endif
