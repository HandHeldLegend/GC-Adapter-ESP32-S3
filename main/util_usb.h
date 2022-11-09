#ifndef UTIL_USB_H
#define UTIL_USB_H

#include "adapter_includes.h"

#define CFG_TUD_VENDOR          1

typedef enum
{
    USB_MODE_NS,
    USB_MODE_GC,
    //USB_MODE_GENERIC,
    USB_MODE_MAX,
} usb_mode_t;

typedef enum
{
    GCSTATUS_IDLE,
    GCSTATUS_WORKING,
} adapter_status_t;

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

extern usb_mode_t adapter_mode;
extern adapter_status_t adapter_status;

uint8_t dir_to_hat(uint8_t leftRight, uint8_t upDown);

esp_err_t gcusb_start(usb_mode_t mode);

void gcusb_stop();

void gcusb_send_data();

#endif