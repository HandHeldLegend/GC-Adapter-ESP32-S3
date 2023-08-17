#ifndef UTIL_USB_H
#define UTIL_USB_H

#include "adapter_includes.h"

typedef enum
{
  XI_HAT_TOP          = 0x01,
  XI_HAT_TOP_RIGHT    = 0x02,
  XI_HAT_RIGHT        = 0x03,
  XI_HAT_BOTTOM_RIGHT = 0x04,
  XI_HAT_BOTTOM       = 0x05,
  XI_HAT_BOTTOM_LEFT  = 0x06,
  XI_HAT_LEFT         = 0x07,
  XI_HAT_TOP_LEFT     = 0x08,
  XI_HAT_CENTER       = 0x00,
} xi_input_hat_dir_t;

#define USB_TIMEOUT_CAP 200
extern usb_mode_t adapter_mode;
extern uint16_t usb_timeout_time;
extern uint8_t usb_polling_rate;
extern const char* global_string_descriptor[];

uint8_t dir_to_hat(uint8_t leftRight, uint8_t upDown);
uint8_t scale_axis(int input);
short sign_axis(int input);
uint8_t scale_trigger(int input);
uint8_t gc_origin_adjust(uint8_t value, int origin, bool invert);

void gcusb_start(usb_mode_t mode);

void gc_reset_data(void);

void usb_send_data(void);

void rmt_reset(void);

void usb_process_data(void);

#endif