#include "util_usb.h"

usb_mode_t adapter_mode         = USB_MODE_NS;
adapter_status_t adapter_status = GCSTATUS_IDLE;

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

const uint8_t ns_hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)

    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x05, 0x09,        //   Usage Page (Button)

    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)

    0x25, 0x07,        //   Logical Maximum (7)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x09, 0x39,        //   Usage (Hat switch)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x46, 0xFF, 0x00,  //   Physical Maximum (255)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)

    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x0C,        //   Usage Page (Consumer)

    0x09, 0x00,        //   Usage (Unassigned)
    0x15, 0x80,        //   Logical Minimum (-128)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0xC0,              // End Collection
    // 94 bytes
};

// Generic Gamepad HID descriptor
const uint8_t hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)

    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x05, 0x09,        //   Usage Page (Button)

    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)

    0x25, 0x07,        //   Logical Maximum (7)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x09, 0x39,        //   Usage (Hat switch)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x46, 0xFF, 0x00,  //   Physical Maximum (255)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)

    // EXPERIMENTAL ADDITIONS
    0x09, 0x33,
    0x09, 0x34,

    0x75, 0x08,        //   Report Size (8)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x0C,        //   Usage Page (Consumer)

    0x09, 0x00,        //   Usage (Unassigned)
    0x15, 0x80,        //   Logical Minimum (-128)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0xC0,              // End Collection
    // 94 bytes
};

const uint8_t gc_hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x11,        //     Report ID (17)
    0x19, 0x00,        //     Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //     Usage Maximum (0xFF)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x05,        //     Report Count (5)
    0x91, 0x00,        //     Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x21,        //     Report ID (33)
    0x05, 0x00,        //     Usage Page (Undefined)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0xFF,        //     Logical Maximum (-1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x08,        //     Usage Maximum (0x08)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x32,        //     Usage (Z)
    0x09, 0x33,        //     Usage (Rx)
    0x09, 0x34,        //     Usage (Ry)
    0x09, 0x35,        //     Usage (Rz)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x06,        //     Report Count (6)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x13,        //     Report ID (19)
    0x19, 0x00,        //     Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //     Usage Maximum (0xFF)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x00,        //     Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

const uint8_t xi_hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)

    0x85, 0x01,        //   Report ID (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x05, 0x09,        //   Usage Page (Button)

    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)

    0x25, 0x07,        //   Logical Maximum (7)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x09, 0x39,        //   Usage (Hat switch)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x46, 0xFF, 0x00,  //   Physical Maximum (255)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)

    // EXPERIMENTAL ADDITIONS
    0x09, 0x33,
    0x09, 0x34,

    0x75, 0x08,        //   Report Size (8)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x0C,        //   Usage Page (Consumer)

    0x09, 0x00,        //   Usage (Unassigned)
    0x15, 0x80,        //   Logical Minimum (-128)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    // End of Input part

    // RUMBLE REPORTS
    0x05, 0x0F,        //   Usage Page (PID Page)
    0x09, 0x21,        //   Usage (0x21)

    // Byte 1 report ID
    0x85, 0x03,        //   Report ID (3)

    // Byte 2 function
    // 0x0F = Enable Actuators
    0xA1, 0x02,        //   Collection (Logical)
    0x09, 0x97,        //     Usage (0x97)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x04,        //     Report Size (4)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x00,        //     Logical Maximum (0)
    0x75, 0x04,        //     Report Size (4)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x03,        //     Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0x09, 0x70,        //     Usage (0x70)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x64,        //     Logical Maximum (100)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x04,        //     Report Count (4)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x09, 0x50,        //     Usage (0x50)
    0x66, 0x01, 0x10,  //     Unit (System: SI Linear, Time: Seconds)
    0x55, 0x0E,        //     Unit Exponent (-2)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    
    0x09, 0xA7,        //     Usage (0xA7)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x65, 0x00,        //     Unit (None)
    0x55, 0x00,        //     Unit Exponent (0)

    0x09, 0x7C,        //     Usage (0x7C)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection

    // Battery level output

    0x85, 0x04,        //   Report ID (4)
    0x05, 0x06,        //   Usage Page (Generic Dev Ctrls)
    0x09, 0x20,        //   Usage (Battery Strength)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection

    // 306 bytes
};

/**** GameCube Adapter Device Descriptor ****/
static const tusb_desc_device_t gc_descriptor_dev = {
    .bLength = 18,
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x057E,
    .idProduct = 0x0337,

    .bcdDevice = CONFIG_TINYUSB_DESC_BCD_DEVICE,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

/**** xInput Device Descriptor ****/
static const tusb_desc_device_t xi_descriptor_dev = {
    .bLength = 18,
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0xFF,
    .bDeviceSubClass = 0xFF,
    .bDeviceProtocol = 0xFF,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x045E,
    .idProduct = 0x02E0,

    .bcdDevice = 0x0114,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

/**** GameCube Adapter Configuration Descriptor ****/

#define GC_CGCDES_LEN   9 + 9 + 9 + 7 + 7
static const uint8_t gc_hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, 41, TUSB_DESC_CONFIG_ATT_SELF_POWERED, 500),
    // Interface
    9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_HID, 0x00, 0x00, 0x00,
    // HID Descriptor
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(sizeof(gc_hid_report_descriptor)),
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x81, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(37), 1,
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(6), 8,
};  

static const tinyusb_config_t gc_cfg = {
                .device_descriptor = &gc_descriptor_dev,
                .string_descriptor = NULL,
                .external_phy = false,
                .configuration_descriptor = gc_hid_configuration_descriptor,
            };

/**** xInput Configuration Descriptor ****/
#define XI_CGCDES_LEN   9 + 9 + 9 + 7 + 7
static const uint8_t xinput_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, 41, TUSB_DESC_CONFIG_ATT_SELF_POWERED, 500),
    // Interface
    9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_HID, 0x00, 0x00, 0x00,
    // HID Descriptor
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(sizeof(xi_hid_report_descriptor)),
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x81, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(38), 1,
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(6), 8,
};

tusb_desc_strarray_device_t xi_descriptor_str = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},                // 0: is supported language is English (0x0409)
    "Microsoft", // 1: Manufacturer
    "Xbox 360 Controller",      // 2: Product
    CONFIG_TINYUSB_DESC_SERIAL_STRING,       // 3: Serials, should use chip ID
};

static const tinyusb_config_t xi_cfg = {
                .device_descriptor = &xi_descriptor_dev,
                .string_descriptor = &xi_descriptor_str,
                .external_phy = false,
                .configuration_descriptor = xinput_configuration_descriptor,
};

static const uint8_t ns_hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(ns_hid_report_descriptor), 0x81, 8, 1),
};

static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(hid_report_descriptor), 0x81, 10, 1),
};

/********* TinyUSB HID callbacks ***************/
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) reqlen;

  return 0;
}

bool first = false;
bool usb_clear = true;
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{   
    switch (adapter_mode)
    {
        default:
        case USB_MODE_NS:
        case USB_MODE_GENERIC:
        case USB_MODE_XINPUT:
            usb_clear = true;
            break;
        case USB_MODE_GC:
            if (report[0] == 0x21)
            {
                usb_clear = true;
            }
            break;
    }
    
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    switch (adapter_mode)
    {
        default:
        case USB_MODE_NS:
            
            break;
        case USB_MODE_GC:
            if (!report_id && !report_type)
            {
                if (buffer[0] == 0x11)
                {
                    rx_vibrate = buffer[1] & 0x1;
                }
            }
            break;
        /*case USB_MODE_GENERIC:
            //return generic_hid_report_descriptor;
            break;*/
    }  
}

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void) instance;
    switch (adapter_mode)
    {
        default:
        case USB_MODE_NS:
            return ns_hid_report_descriptor;
            break;
        case USB_MODE_GC:
            return gc_hid_report_descriptor;
            break;
        case USB_MODE_GENERIC:
            return hid_report_descriptor;
            break;
        case USB_MODE_XINPUT:
            return xi_hid_report_descriptor;
            break;
    }
    return NULL;
}

uint8_t dir_to_hat(uint8_t leftRight, uint8_t upDown)
{
    uint8_t ret = HAT_CENTER;

    if (leftRight == 2)
    {
        ret = HAT_RIGHT;
        if (upDown == 2)
        {
            ret = HAT_TOP_RIGHT;
        }
        else if (upDown == 0)
        {
            ret = HAT_BOTTOM_RIGHT;
        }
    }
    else if (leftRight == 0)
    {
        ret = HAT_LEFT;
        if (upDown == 2)
        {
            ret = HAT_TOP_LEFT;
        }
        else if (upDown == 0)
        {
            ret = HAT_BOTTOM_LEFT;
        }
    }

    else if (upDown == 2)
    {
        ret = HAT_TOP;
    }
    else if (upDown == 0)
    {
        ret = HAT_BOTTOM;
    }

    return ret;
}

#define ANALOG_SCALER (float) 1.28
uint8_t scale_axis(int input)
{
    int res = input;
    if (input > 130)
    {
        float tmp = (float) input - 128;
        tmp = tmp * ANALOG_SCALER;
        res = (int) tmp + 128;
    }
    else if (input < 126)
    {
        float tmp = 128 - (float) input;
        tmp = tmp * ANALOG_SCALER;
        res = 128 - (int) tmp;    
    }
    
    if (res > 255)
    {
        res = 255;
    }
    if (res < 0)
    {
        res = 0;
    }
    return (uint8_t) res;
}

esp_err_t gcusb_start(usb_mode_t mode)
{
    const char* TAG = "gcusb_start";

    ESP_LOGI(TAG, "USB initialization");

    switch (mode)
    {
        default:
        case USB_MODE_NS:
            ESP_LOGI(TAG, "NS MODE");

            const tinyusb_config_t ns_cfg = {
                .device_descriptor = NULL,
                .string_descriptor = NULL,
                .external_phy = false,
                .configuration_descriptor = ns_hid_configuration_descriptor,
            };

            ESP_ERROR_CHECK(tinyusb_driver_install(&ns_cfg));
            break;
        case USB_MODE_GENERIC:
            ESP_LOGI(TAG, "HID MODE");
            
            const tinyusb_config_t hid_cfg = {
                .device_descriptor = NULL,
                .string_descriptor = NULL,
                .external_phy = false,
                .configuration_descriptor = hid_configuration_descriptor,
            };

            ESP_ERROR_CHECK(tinyusb_driver_install(&hid_cfg));
            break;
        case USB_MODE_GC:
            ESP_LOGI(TAG, "GCC MODE");

            ESP_ERROR_CHECK(tinyusb_driver_install(&gc_cfg));
            break;

        case USB_MODE_XINPUT:
            ESP_LOGI(TAG, "XINPUT MODE");

            ESP_ERROR_CHECK(tinyusb_driver_install(&xi_cfg));
            break;
    }

    adapter_status = GCSTATUS_WORKING;
    return ESP_OK;
}

void gcusb_stop()
{
    adapter_status = GCSTATUS_IDLE;
}

int adj_x;
int adj_y;
int adj_cx;
int adj_cy;

void gcusb_send_data(bool repeat)
{
    if (adapter_status == GCSTATUS_IDLE)
    {
        return;
    }

    switch (adapter_mode)
    {
        default:
        case USB_MODE_GENERIC:
        case USB_MODE_NS:
            if (!repeat)
            {
                // Generate the USB Data for NS mode
                ns_input.button_a = gc_poll_response.button_a;
                ns_input.button_b = gc_poll_response.button_b;
                ns_input.button_x = gc_poll_response.button_x;
                ns_input.button_y = gc_poll_response.button_y;

                uint8_t lr = 1 - gc_poll_response.dpad_left + gc_poll_response.dpad_right;
                uint8_t ud = 1 - gc_poll_response.dpad_down + gc_poll_response.dpad_up;

                ns_input.dpad_hat = dir_to_hat(lr, ud);

                ns_input.button_plus = gc_poll_response.button_start;

                ns_input.trigger_r = gc_poll_response.button_z;

                ns_input.trigger_zl = gc_poll_response.button_l;
                ns_input.trigger_zr = gc_poll_response.button_r;
                
                adj_x   = (int) gc_poll_response.stick_x + gc_origin_data.stick_x;
                adj_y   = 256 - (int) gc_poll_response.stick_y + gc_origin_data.stick_y;
                adj_cx  = (int) gc_poll_response.cstick_x + gc_origin_data.cstick_x;
                adj_cy  = 256 - (int) gc_poll_response.cstick_y + gc_origin_data.cstick_y;

                ns_input.stick_left_x   = scale_axis(adj_x);
                ns_input.stick_left_y   = scale_axis(adj_y);
                ns_input.stick_right_x  = scale_axis(adj_cx);
                ns_input.stick_right_y  = scale_axis(adj_cy);

                if (adapter_mode == USB_MODE_GENERIC)
                {
                    ns_input.analog_trigger_l  = gc_poll_response.trigger_l;
                    ns_input.analog_trigger_r  = gc_poll_response.trigger_r;
                }
                else
                {
                    ns_input.analog_trigger_l = 0x00;
                    ns_input.analog_trigger_r = 0x00;
                }
                
            }
            
            if (adapter_mode == USB_MODE_NS)
            {
                memcpy(&ns_buffer, &ns_input, NS_HID_LEN);
                // Send USB report
                if (usb_clear)
                {
                    usb_clear = false;
                    tud_hid_report(0, &ns_buffer, NS_HID_LEN);
                }
            }
            else if (adapter_mode == USB_MODE_GENERIC)
            {
                memcpy(&hid_buffer, &ns_input, GP_HID_LEN);
                // Send USB report
                if (usb_clear)
                {
                    usb_clear = false;
                    tud_hid_report(0, &hid_buffer, GP_HID_LEN);
                }
            }
            
            break;
        case USB_MODE_GC:
            if (!repeat)
            {
                // Generate the USB Data for GameCube native mode
                gc_input.button_a   = gc_poll_response.button_a;
                gc_input.button_b   = gc_poll_response.button_b;
                gc_input.button_x   = gc_poll_response.button_x;
                gc_input.button_y   = gc_poll_response.button_y;

                gc_input.dpad_down  = gc_poll_response.dpad_down;
                gc_input.dpad_up    = gc_poll_response.dpad_up;
                gc_input.dpad_left  = gc_poll_response.dpad_left;
                gc_input.dpad_right = gc_poll_response.dpad_right;

                gc_input.button_start   = gc_poll_response.button_start;
                gc_input.button_z       = gc_poll_response.button_z;
                gc_input.button_r       = gc_poll_response.button_r;
                gc_input.button_l       = gc_poll_response.button_l;

                gc_input.trigger_l      = gc_poll_response.trigger_l;
                gc_input.trigger_r      = gc_poll_response.trigger_r;

                adj_x   = (int) gc_poll_response.stick_x + gc_origin_data.stick_x;
                adj_y   = (int) gc_poll_response.stick_y + gc_origin_data.stick_y;
                adj_cx  = (int) gc_poll_response.cstick_x + gc_origin_data.cstick_x;
                adj_cy  = (int) gc_poll_response.cstick_y + gc_origin_data.cstick_y;

                gc_input.stick_x    = (uint8_t) adj_x;
                gc_input.stick_y    = (uint8_t) adj_y;
                gc_input.cstick_x   = (uint8_t) adj_cx;
                gc_input.cstick_y   = (uint8_t) adj_cy;
            }

            if (first)
            {
                memcpy(&gc_buffer[2], &gc_input, 8);
            }
            
            gc_buffer[0] = 0x21;
            gc_buffer[1] = 0x10;

            if (usb_clear)
            {
                usb_clear = false;
                tud_hid_report(0, &gc_buffer, GC_HID_LEN); 
            }

            first = true;
            
            break;

        case USB_MODE_XINPUT:
            xi_input.report_id = 0x01;
            if (!repeat)
            {

                if (gc_poll_response.button_start && gc_poll_response.button_a)
                {
                    xi_input.button_start = 1;
                }
                else if (gc_poll_response.button_start && gc_poll_response.button_b)
                {
                    xi_input.button_back = 1;
                }
                else if (gc_poll_response.button_start && gc_poll_response.button_z)
                {
                    xi_input.button_guide = 1;
                }
                else
                {
                    xi_input.button_a = gc_poll_response.button_a;
                    xi_input.button_b = gc_poll_response.button_b;
                    xi_input.button_x = gc_poll_response.button_x;
                    xi_input.button_y = gc_poll_response.button_y;
                    xi_input.bumper_r = gc_poll_response.button_z;

                    xi_input.button_start   = 0;
                    xi_input.button_back    = 0;
                    xi_input.button_guide   = 0;
                }

                xi_input.dpad_up = gc_poll_response.dpad_up;
                xi_input.dpad_left = gc_poll_response.dpad_left;
                xi_input.dpad_down = gc_poll_response.dpad_down;
                xi_input.dpad_right = gc_poll_response.dpad_right;

                xi_input.trigger_l = gc_poll_response.button_l;
                xi_input.trigger_r = gc_poll_response.button_r;

                adj_x   = (int) gc_poll_response.stick_x + gc_origin_data.stick_x;
                adj_y   = 256 - (int) gc_poll_response.stick_y + gc_origin_data.stick_y;
                adj_cx  = (int) gc_poll_response.cstick_x + gc_origin_data.cstick_x;
                adj_cy  = 256 - (int) gc_poll_response.cstick_y + gc_origin_data.cstick_y;

                xi_input.stick_left_x   = scale_axis(adj_x);
                xi_input.stick_left_y   = scale_axis(adj_y);
                xi_input.stick_right_x  = scale_axis(adj_cx);
                xi_input.stick_right_y  = scale_axis(adj_cy);

                xi_input.analog_trigger_l = 0x0000;
                xi_input.analog_trigger_r = 0x0000;

                uint8_t lr = 1 - gc_poll_response.dpad_left + gc_poll_response.dpad_right;
                uint8_t ud = 1 - gc_poll_response.dpad_down + gc_poll_response.dpad_up;
                
            }
            memcpy(&xi_buffer, &xi_input, XI_HID_LEN);
            // Send USB report
            if (usb_clear)
            {
                usb_clear = false;
                tud_hid_report(0, &xi_buffer, XI_HID_LEN);
            }
    }
}