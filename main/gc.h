#ifndef GC_H
#define GC_H
#include "rmt_gc.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "freertos/semphr.h"

#define GCMD_PROBE_LEN  10
#define GCMD_ORIGIN_LEN 10
#define GCMD_POLL_LEN   26
#define RX_TIMEOUT_THRESH   5000

extern QueueHandle_t usb_queue;

extern rmt_item32_t gcmd_probe_rmt[GCMD_PROBE_LEN];
extern rmt_item32_t gcmd_origin_rmt[GCMD_ORIGIN_LEN];
extern rmt_item32_t gcmd_poll_rmt[GCMD_POLL_LEN];

typedef enum
{
    CMD_PHASE_PROBE,
    CMD_PHASE_ORIGIN,
    CMD_PHASE_POLL
} gc_cmd_phase_t;

extern gc_cmd_phase_t cmd_phase;

typedef enum
{
    GC_TYPE_WIRED,
    GC_TYPE_KEYBOARD,
    GC_TYPE_UNKNOWN
} gc_type_t;

#define GC_PROBE_RESPONSE_ITEMS    

typedef struct
{
    uint8_t id_upper;
    uint8_t id_lower;
    uint8_t junk;
} gc_probe_response_s;

extern gc_probe_response_s gc_probe_response;

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
            uint8_t b1blank         : 3;  
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t button_dl       : 1;
            uint8_t button_dr       : 1;
            uint8_t button_dd       : 1;
            uint8_t button_du       : 1;
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
} gc_poll_response_s;

extern gc_poll_response_s gc_poll_response;

typedef struct
{
    bool    data_set;
    int stick_x;
    int stick_y;
    int cstick_x;
    int cstick_y;
} gc_origin_data_s;

extern gc_origin_data_s gc_origin_data;

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

typedef enum
{
  HAT_TOP          = 0x00,
  HAT_TOP_RIGHT    = 0x01,
  HAT_RIGHT        = 0x02,
  HAT_BOTTOM_RIGHT = 0x03,
  HAT_BOTTOM       = 0x04,
  HAT_BOTTOM_LEFT  = 0x05,
  HAT_LEFT         = 0x06,
  HAT_TOP_LEFT     = 0x07,
  HAT_CENTER       = 0x08,
} input_hat_dir_t;

typedef struct
{
    uint8_t phase;
} xmessage_s;

extern uint8_t usb_buffer[8];
extern volatile uint32_t rx_timeout;
extern volatile bool rx_recieved; 

esp_err_t gamecube_reader_start();

void gamecube_send_usb(void);

#endif