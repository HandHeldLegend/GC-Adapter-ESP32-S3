#ifndef GC_H
#define GC_H

#include "adapter_includes.h"
#include "driver/rmt_types_legacy.h"

#define GCMD_PROBE_LEN  10
#define GCMD_ORIGIN_LEN 10
#define GCMD_POLL_LEN   26
#define RX_TIMEOUT_THRESH   5000

#define GC_PROBE_RESPONSE_LEN   25
#define GC_ORIGIN_RESPONSE_LEN  81
#define GC_POLL_RESPONSE_LEN    65
#define GC_MEM_OFFSET           0xC0

// The index of the RMT memory where the outgoing vibrate bit is set
#define GC_POLL_VIBRATE_IDX    23  

extern rmt_item32_t gcmd_probe_rmt[GCMD_PROBE_LEN];
extern rmt_item32_t gcmd_origin_rmt[GCMD_ORIGIN_LEN];
extern rmt_item32_t gcmd_poll_rmt[GCMD_POLL_LEN];

extern gc_cmd_phase_t cmd_phase;
extern gc_usb_phase_t usb_phase;
extern usb_mode_t active_usb_mode;

typedef struct
{
    uint8_t id_upper;
    uint8_t id_lower;
    uint8_t junk;
} __attribute__ ((packed)) gc_probe_response_s;

extern gc_probe_response_s  gc_probe_response;

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

extern volatile uint32_t    rx_timeout;
extern volatile uint32_t   rx_timeout_counts;
extern volatile bool        rx_recieved;
extern volatile uint32_t    rx_offset;
extern volatile bool rx_vibrate;

// Hold the origin data for a session
extern gc_origin_data_s     gc_origin_data;

esp_err_t gamecube_rmt_init(void);

void gamecube_rmt_process(void);

void adapter_mode_task(void *param);

#endif