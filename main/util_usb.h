#ifndef UTIL_USB_H
#define UTIL_USB_H

#include "adapter_includes.h"

typedef enum
{
    USB_MODE_GENERIC  = 0x00,
    USB_MODE_NS       = 0x01,
    USB_MODE_XINPUT   = 0x02,
    USB_MODE_GC       = 0x03,
    USB_MODE_MAX,
} usb_mode_t;

typedef enum
{
    GCSTATUS_IDLE,
    GCSTATUS_WORKING,
} adapter_status_t;

typedef enum
{
  NS_HAT_TOP          = 0x00,
  NS_HAT_TOP_RIGHT    = 0x01,
  NS_HAT_RIGHT        = 0x02,
  NS_HAT_BOTTOM_RIGHT = 0x03,
  NS_HAT_BOTTOM       = 0x04,
  NS_HAT_BOTTOM_LEFT  = 0x05,
  NS_HAT_LEFT         = 0x06,
  NS_HAT_TOP_LEFT     = 0x07,
  NS_HAT_CENTER       = 0x08,
} ns_input_hat_dir_t;

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


typedef enum
{
    HAT_MODE_NS,
    HAT_MODE_XI,
} hat_mode_t;

extern usb_mode_t adapter_mode;
extern adapter_status_t adapter_status;

uint8_t dir_to_hat(hat_mode_t hat_type, uint8_t leftRight, uint8_t upDown);

esp_err_t gcusb_start(usb_mode_t mode);

void gcusb_stop();

void gcusb_send_data(bool repeat);

#endif