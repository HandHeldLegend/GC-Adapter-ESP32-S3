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
extern gc_type_t active_gc_type;

extern gc_probe_response_s  gc_probe_response;

extern volatile uint32_t    rx_timeout;
extern volatile uint32_t   rx_timeout_counts;
extern volatile bool        rx_recieved;
extern volatile uint32_t    rx_offset;
extern volatile bool rx_vibrate;

extern float analog_scaler_f;

// Hold the origin data for a session
extern gc_origin_data_s     gc_origin_data;

void gamecube_rumble_en(bool enable);

void gamecube_convert_analog_scaler(void);

esp_err_t gamecube_rmt_init(void);

void gamecube_rmt_process(void);

void adapter_mode_task(void *param);

#endif