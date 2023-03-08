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
    GC_TYPE_WIRED,
    GC_TYPE_KEYBOARD,
    GC_TYPE_UNKNOWN
} gc_type_t;

typedef enum
{
    USB_MODE_GENERIC  = 0x00,
    USB_MODE_NS       = 0x01,
    USB_MODE_XINPUT   = 0x02,
    USB_MODE_GC       = 0x03,
    USB_MODE_MAX,
} usb_mode_t;

#endif