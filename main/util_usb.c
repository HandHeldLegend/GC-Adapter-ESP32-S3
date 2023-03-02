#include "util_usb.h"

usb_mode_t adapter_mode         = USB_MODE_NS;
adapter_status_t adapter_status = GCSTATUS_IDLE;

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

tusb_desc_strarray_device_t global_string_descriptor = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},                // 0: is supported language is English (0x0409)
    "HHL", // 1: Manufacturer
    "GC Pocket Adapter",      // 2: Product
    CONFIG_TINYUSB_DESC_SERIAL_STRING,       // 3: Serials, should use chip ID
};

/** DINPUT HID MODE **/
// 0. String Descriptor
// 1. Device Descriptor
// 2. HID Report Descriptor
// 3. Configuration Descriptor
// 4. TinyUSB Config
/**--------------------------**/

static const tusb_desc_device_t di_device_descriptor = {
    .bLength = 18,
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x20d6,
    .idProduct = 0xa714,

    .bcdDevice = CONFIG_TINYUSB_DESC_BCD_DEVICE,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

// Generic Gamepad HID descriptor
const uint8_t dinput_hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)

    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
        0xA1, 0x01,         // Collection (Application)
            0x85, 0x01,        //   Report ID (1)

            0x05, 0x09,        //   Usage Page (Button)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x01,        //   Logical Maximum (1)
            0x35, 0x00,        //   Physical Minimum (0)
            0x45, 0x01,        //   Physical Maximum (1)
            0x75, 0x01,        //   Report Size (1)
            0x95, 0x0E,        //   Report Count (14)
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
            0x09, 0x33,        //   Usage (Rx)
            0x09, 0x34,        //   Usage (Ry)

            0x75, 0x08,        //   Report Size (8)
            0x95, 0x06,        //   Report Count (6)
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

        0xc0,

        0xA1, 0x01,         // Collection (Application)
            0x06, 0x00, 0xFF,      //            USAGE_PAGE (Vendor Defined Page 1) 
            0x09, 0x01,            //            USAGE (Vendor Usage 1) 
            0x85, 0x02,            //           Report ID (2)
            0x15, 0x00,            //            LOGICAL_MINIMUM (0) 
            0x26, 0xff, 0x00,       //            LOGICAL_MAXIMUM (255) 
            0x75, 0x08,            //            REPORT_SIZE (8) 
            0x95, 0x09,            //            REPORT_COUNT (11) 
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xc0,

        0xA1, 0x01,         // Collection (Application)
            0x06, 0x00, 0xFF,      //            USAGE_PAGE (Vendor Defined Page 1) 
            0x09, 0x01,            //            USAGE (Vendor Usage 1) 
            0x85, 0x02,            //           Report ID (2)
            0x15, 0x00,            //            LOGICAL_MINIMUM (0) 
            0x26, 0xff, 0x00,       //            LOGICAL_MAXIMUM (255) 
            0x75, 0x08,            //            REPORT_SIZE (8) 
            0x95, 0x0B,            //            REPORT_COUNT (11) 
            0x91, 0x02,            //            OUTPUT (Data,Var,Abs)
        0xc0,
    // 125 bytes
    0xC0
};

static const uint8_t di_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, 41, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),
    // Interface
    9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_HID, 0x00, 0x00, 0x00,
    // HID Descriptor
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(sizeof(dinput_hid_report_descriptor)),
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x81, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(64), 1,
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(13), 1,
};

const tinyusb_config_t di_cfg = {
    .device_descriptor = &di_device_descriptor,
    .string_descriptor = global_string_descriptor,
    .external_phy = false,
    .configuration_descriptor = di_configuration_descriptor,
};
/**--------------------------**/
/**--------------------------**/


/** NS HID MODE **/
// 0. String Descriptor
// 1. Device Descriptor
// 2. HID Report Descriptor
// 3. Configuration Descriptor
// 4. TinyUSB Config
/**--------------------------**/

static const tusb_desc_device_t ns_device_descriptor = {
    .bLength = 18,
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x20d6,
    .idProduct = 0xa713,

    .bcdDevice = CONFIG_TINYUSB_DESC_BCD_DEVICE,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

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

static const uint8_t ns_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(ns_hid_report_descriptor), 0x81, 8, 1),
};

const tinyusb_config_t ns_cfg = {
    .device_descriptor = &ns_device_descriptor,
    .string_descriptor = global_string_descriptor,
    .external_phy = false,
    .configuration_descriptor = ns_configuration_descriptor,
};
/**--------------------------**/
/**--------------------------**/



/** GAMECUBE HID MODE **/
// 1. Device Descriptor
// 2. HID Report Descriptor
// 3. Configuration Descriptor
// 4. TinyUSB Config
/**--------------------------**/

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

/**** GameCube Adapter HID Report Descriptor ****/
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
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(6), 1,
};  

/**** GameCube Adapter TinyUSB Config ****/
static const tinyusb_config_t gc_cfg = {
    .device_descriptor          = &gc_descriptor_dev,
    .string_descriptor          = global_string_descriptor,
    .external_phy               = false,
    .configuration_descriptor   = gc_hid_configuration_descriptor,
};

/**--------------------------**/
/**--------------------------**/


/** XBOX HID MODE (NOT QUITE XINPUT) **/
// 1. Device Descriptor
// 2. HID Report Descriptor
// 3. Configuration Descriptor
// 4. TinyUSB Config
/**--------------------------**/

/**** xInput Device Descriptor ****/
static const tusb_desc_device_t xi_device_descriptor = {
    .bLength = 18,
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_HID,
    .bDeviceSubClass = 0xFF,
    .bDeviceProtocol = 0xFF,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x045E,
    .idProduct = 0x0b20,
    .idProduct = 0xB13,

    .bcdDevice = 0x0572,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

// XInput HID Descriptor
const uint8_t xinput_hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)

    // Byte 0
    0x85, 0x01,        //   Report ID (1)

    // Bytes 1, 2, 3, 4 for left stick X and Y
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x00,        //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
    0x95, 0x02,        //     Report Count (2)
    0x75, 0x10,        //     Report Size (16)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection

    // Bytes 4, 5, 6, 7 for right stick X and Y
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x09, 0x33,        //     Usage (Rx)
    0x09, 0x34,        //     Usage (Ry)
    0x15, 0x00,        //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
    0x95, 0x02,        //     Report Count (2)
    0x75, 0x10,        //     Report Size (16)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection

    // Bytes 8, 9 for left analog trigger
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x32,        //   Usage (Z)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x0A,        //   Report Size (10)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x00,        //   Logical Maximum (0)
    0x75, 0x06,        //   Report Size (6)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    // Bytes 10, 11 for right analog trigger
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x0A,        //   Report Size (10)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x00,        //   Logical Maximum (0)
    0x75, 0x06,        //   Report Size (6)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    // Byte 12 for Hat switch for dpad 
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x01,        //   Logical Minimum (1)
    0x25, 0x08,        //   Logical Maximum (8)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x66, 0x14, 0x00,  //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x00,        //   Logical Maximum (0)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x00,        //   Physical Maximum (0)
    0x65, 0x00,        //   Unit (None)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    // Bytes 13 and 14 for buttons_1
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0A,        //   Usage Maximum (0x0A)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0A,        //   Report Count (10)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x00,        //   Logical Maximum (0)
    0x75, 0x06,        //   Report Size (6)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x80,        //   Usage (Sys Control)
    0x85, 0x02,        //   Report ID (2)
    0xA1, 0x00,        //   Collection (Physical)
    0x09, 0x85,        //     Usage (Sys Main Menu)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x00,        //     Logical Maximum (0)
    0x75, 0x07,        //     Report Size (7)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection

    0x05, 0x0F,        //   Usage Page (PID Page)
    0x09, 0x21,        //   Usage (0x21)

    0x85, 0x03,        //   Report ID (3)

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

/**** xInput Configuration Descriptor ****/
#define XI_CGCDES_LEN   9 + 9 + 9 + 7 + 7
static const uint8_t xi_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, 41, TUSB_DESC_CONFIG_ATT_SELF_POWERED, 500),
    // Interface
    9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_HID, 0x00, 0x00, 0x00,
    // HID Descriptor
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(sizeof(xinput_hid_report_descriptor)),
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x81, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(32), 4,
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(33), 4,
};

static const tinyusb_config_t xi_cfg = {
    .device_descriptor          = &xi_device_descriptor,
    .string_descriptor          = global_string_descriptor,
    .external_phy               = false,
    .configuration_descriptor   = xi_configuration_descriptor,
};

/**--------------------------**/
/**--------------------------**/




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
    switch (adapter_settings.adapter_mode)
    {
        case USB_MODE_GENERIC:
            if (report[0] == 0x01)
            {
                usb_clear = true;
            }
            break;

        default:
        case USB_MODE_NS:
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
    switch (adapter_settings.adapter_mode)
    {
        default:

            break;
        case USB_MODE_GENERIC:
            if (!report_id && !report_type)
            {
                if (buffer[0] == CMD_USB_REPORTID)
                {
                    command_handler(buffer, bufsize);
                }
            }
            break;

        case USB_MODE_NS:
            
            break;
        case USB_MODE_GC:
            if (!report_id && !report_type)
            {
                if (buffer[0] == 0x11)
                {
                    ESP_LOGI("RUMBLE", "Rx");
                    for (uint8_t x = 0; x < bufsize-1; x++)
                    {
                        ESP_LOGI("d: ", "%X", (unsigned int) buffer[x+1]);
                    }
                    rx_vibrate = (buffer[1] > 0) ? VIBRATE_ON : VIBRATE_OFF;
                }
                else if (buffer[0] == 0x13)
                {
                    ESP_LOGI("INIT", "Rx");
                    adapter_status = GCSTATUS_WORKING;
                }
                else
                {
                    ESP_LOGI("cmd", "Rx: %X", (unsigned int) buffer[0]);
                    for (uint8_t x = 0; x < bufsize-1; x++)
                    {
                        ESP_LOGI("d: ", "%X", (unsigned int) buffer[x+1]);
                    }
                }
            }
            break;
        case USB_MODE_XINPUT:
            if (!report_id && !report_type)
            {
                if (buffer[0] == 0x03 && buffer[1] == 0x0F)
                {
                    xi_rumble_s rumble = {};
                    memcpy(&rumble, buffer, sizeof(xi_rumble_s));
                    if (rumble.magnitude_l > 0 || rumble.magnitude_r > 0)
                    {
                        rx_vibrate = 1;
                    }
                    else
                    {
                        rx_vibrate = 0;
                    }
                }
            }
            break;
    }  
}

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void) instance;
    switch (adapter_settings.adapter_mode)
    {
        default:
        case USB_MODE_XINPUT:
            return xinput_hid_report_descriptor;
            break;
        case USB_MODE_NS:
            return ns_hid_report_descriptor;
            break;
        case USB_MODE_GC:
            return gc_hid_report_descriptor;
            break;
        case USB_MODE_GENERIC:
            return dinput_hid_report_descriptor;
            break;
    }
    return NULL;
}

/**
 * @brief Takes in directions and outputs a byte output appropriate for
 * HID Hat usage.
 * @param hat_type hat_mode_t type - The type of controller you're converting for.
 * @param leftRight 0 through 2 (2 is right) to indicate the direction left/right
 * @param upDown 0 through 2 (2 is up) to indicate the direction up/down
*/
uint8_t dir_to_hat(hat_mode_t hat_type, uint8_t leftRight, uint8_t upDown)
{
    uint8_t ret = 0;
    switch(hat_type)
    {
        default:
        case HAT_MODE_NS:
            ret = NS_HAT_CENTER;

        if (leftRight == 2)
        {
            ret = NS_HAT_RIGHT;
            if (upDown == 2)
            {
                ret = NS_HAT_TOP_RIGHT;
            }
            else if (upDown == 0)
            {
                ret = NS_HAT_BOTTOM_RIGHT;
            }
        }
        else if (leftRight == 0)
        {
            ret = NS_HAT_LEFT;
            if (upDown == 2)
            {
                ret = NS_HAT_TOP_LEFT;
            }
            else if (upDown == 0)
            {
                ret = NS_HAT_BOTTOM_LEFT;
            }
        }

        else if (upDown == 2)
        {
            ret = NS_HAT_TOP;
        }
        else if (upDown == 0)
        {
            ret = NS_HAT_BOTTOM;
        }

        return ret;
        break;

        case HAT_MODE_XI:
                ret = XI_HAT_CENTER;

            if (leftRight == 2)
            {
                ret = XI_HAT_RIGHT;
                if (upDown == 2)
                {
                    ret = XI_HAT_TOP_RIGHT;
                }
                else if (upDown == 0)
                {
                    ret = XI_HAT_BOTTOM_RIGHT;
                }
            }
            else if (leftRight == 0)
            {
                ret = XI_HAT_LEFT;
                if (upDown == 2)
                {
                    ret = XI_HAT_TOP_LEFT;
                }
                else if (upDown == 0)
                {
                    ret = XI_HAT_BOTTOM_LEFT;
                }
            }

            else if (upDown == 2)
            {
                ret = XI_HAT_TOP;
            }
            else if (upDown == 0)
            {
                ret = XI_HAT_BOTTOM;
            }

            return ret;
            break;
    }
}

#define ANALOG_SCALER (float) 1.28
uint8_t scale_axis(int input)
{
    int res = input;
    if (input > 129)
    {
        float tmp = (float) input - 128;
        tmp = tmp * ANALOG_SCALER;
        res = (int) tmp + 128;
    }
    else if (input < 127)
    {
        float tmp = 128 - (float) input;
        tmp = tmp * ANALOG_SCALER;
        res = 128 - (int) tmp;    
    }
    
    if (res >= 255)
    {
        res = 255;
    }
    if (res <= 0)
    {
        res = 0;
    }
    return (uint8_t) res;
}

uint8_t scale_trigger(int input)
{
    if (input < 0)
    {
        return (uint8_t) 0;
    }
    else if ( input <= 255)
    {
        return (uint8_t) input;
    }
    else
    {
        return (uint8_t) 255;
    }
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
            ns_input.stick_left_x = 128;
            ns_input.stick_left_y = 128;
            ns_input.stick_right_x = 128;
            ns_input.stick_right_y = 128;
            ns_input.dpad_hat = NS_HAT_CENTER;
            ESP_ERROR_CHECK(tinyusb_driver_install(&ns_cfg));
            break;
        case USB_MODE_GENERIC:
            ESP_LOGI(TAG, "DINPUT MODE");
            di_input.stick_left_x = 128;
            di_input.stick_left_y = 128;
            di_input.stick_right_x = 128;
            di_input.stick_right_y = 128;
            di_input.analog_trigger_l = 0;
            di_input.analog_trigger_r = 0;
            di_input.dpad_hat = NS_HAT_CENTER;
            ESP_ERROR_CHECK(tinyusb_driver_install(&di_cfg));
            break;
        case USB_MODE_GC:
            gc_input.stick_x = 128;
            gc_input.stick_y = 128;
            gc_input.cstick_x = 128;
            gc_input.cstick_y = 128;
            gc_input.trigger_l = 0;
            gc_input.trigger_r = 0;
            ESP_LOGI(TAG, "GCC MODE");

            ESP_ERROR_CHECK(tinyusb_driver_install(&gc_cfg));
            break;

        case USB_MODE_XINPUT:
            xi_input.stick_left_x = 128;
            xi_input.stick_left_y = 128;
            xi_input.stick_right_x = 128;
            xi_input.stick_right_y = 128;
            xi_input.analog_trigger_l = 0;
            xi_input.analog_trigger_r = 0;
            xi_input.dpad_hat = XI_HAT_CENTER;
            ESP_LOGI(TAG, "XINPUT MODE");

            ESP_ERROR_CHECK(tinyusb_driver_install(&xi_cfg));
            break;
    }

    if (mode != USB_MODE_GC)
    {
        adapter_status = GCSTATUS_WORKING;
    }
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
int adj_tl;
int adj_tr;

void gcusb_send_data(bool repeat)
{
    if (adapter_status == GCSTATUS_IDLE)
    {
        return;
    }

    switch (adapter_settings.adapter_mode)
    {
        default:
        case USB_MODE_GENERIC:
            di_input.report_id = 0x01;
            if (!repeat)
            {
                // Generate the USB Data for NS mode
                di_input.button_a = gc_poll_response.button_a;
                di_input.button_b = gc_poll_response.button_b;
                di_input.button_x = gc_poll_response.button_x;
                di_input.button_y = gc_poll_response.button_y;

                uint8_t lr = 1 - gc_poll_response.dpad_left + gc_poll_response.dpad_right;
                uint8_t ud = 1 - gc_poll_response.dpad_down + gc_poll_response.dpad_up;

                di_input.dpad_hat = dir_to_hat(HAT_MODE_NS, lr, ud);

                di_input.button_plus = gc_poll_response.button_start;

                di_input.trigger_r = gc_poll_response.button_z;

                // Get origin modified Trigger analog data
                adj_tl  = (int) gc_poll_response.trigger_l - gc_origin_data.trigger_l;
                adj_tr  = (int) gc_poll_response.trigger_r - gc_origin_data.trigger_r;

                switch( adapter_settings.di_trigger_l)
                {
                    default:
                    case TRIG_MODE_OFF:
                        di_input.analog_trigger_l   = scale_trigger(adj_tl);
                        di_input.trigger_zl         = gc_poll_response.button_l;
                        break;
                    
                    case TRIG_MODE_A2D:
                        di_input.analog_trigger_l   = (adj_tl >= adapter_settings.trigger_threshold_l) ? 255 : scale_trigger(adj_tl);
                        di_input.trigger_zl         = (adj_tl >= adapter_settings.trigger_threshold_l) ? 1 : 0;
                        break;

                    case TRIG_MODE_D2ALITE:
                        di_input.analog_trigger_l   = (gc_poll_response.button_l) ? 85 : 0;
                        di_input.trigger_zl         = 0;
                        break;
                    
                    case TRIG_MODE_D2AFULL:
                        di_input.analog_trigger_l   = (gc_poll_response.button_l) ? 255 : 0;
                        di_input.trigger_zl         = gc_poll_response.button_l;
                        break;
                }

                switch( adapter_settings.di_trigger_r)
                {
                    default:
                    case TRIG_MODE_OFF:
                        di_input.analog_trigger_r   = scale_trigger(adj_tr);
                        di_input.trigger_zr         = gc_poll_response.button_r;
                        break;
                    
                    case TRIG_MODE_A2D:
                        di_input.analog_trigger_r   = (adj_tr >= adapter_settings.trigger_threshold_r) ? 255 : scale_trigger(adj_tr);
                        di_input.trigger_zr         = (adj_tr >= adapter_settings.trigger_threshold_r) ? 1 : 0;
                        break;

                    case TRIG_MODE_D2ALITE:
                        di_input.analog_trigger_r   = (gc_poll_response.button_r) ? 85 : 0;
                        di_input.trigger_zr         = 0;
                        break;
                    
                    case TRIG_MODE_D2AFULL:
                        di_input.analog_trigger_r   = (gc_poll_response.button_r) ? 255 : 0;
                        di_input.trigger_zr         = gc_poll_response.button_r;
                        break;
                }
                
                adj_x   = (int) gc_poll_response.stick_x - gc_origin_data.stick_x;
                adj_y   = (int) gc_poll_response.stick_y - gc_origin_data.stick_y;
                adj_cx  = (int) gc_poll_response.cstick_x - gc_origin_data.cstick_x;
                adj_cy  = (int) gc_poll_response.cstick_y - gc_origin_data.cstick_y;

                di_input.stick_left_x   = (uint8_t) adj_x;
                di_input.stick_left_y   = (uint8_t) adj_y;
                di_input.stick_right_x  = (uint8_t) adj_cx;
                di_input.stick_right_y  = (uint8_t) adj_cy;
                
            }

            memcpy(&di_buffer, &di_input, DI_HID_LEN);
            // Send USB report
            if (usb_clear && tud_hid_ready())
            {
                usb_clear = false;
                tud_hid_report(0, &di_buffer, DI_HID_LEN);
            }
            break;
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

                ns_input.dpad_hat = dir_to_hat(HAT_MODE_NS, lr, ud);

                uint32_t regread = REG_READ(GPIO_IN_REG) & PIN_MASK_GCP;
                ns_input.button_home = !util_getbit(regread, HOME_BUTTON);
                ns_input.button_capture = !util_getbit(regread, APP_BUTTON);

                ns_input.button_plus = gc_poll_response.button_start;

                ns_input.trigger_r = gc_poll_response.button_z;

                adj_tl  = (int) gc_poll_response.trigger_l - gc_origin_data.trigger_l;
                adj_tr  = (int) gc_poll_response.trigger_r - gc_origin_data.trigger_r;

                switch( adapter_settings.ns_trigger_l)
                {
                    default:
                    case TRIG_MODE_OFF:
                        ns_input.trigger_zl = gc_poll_response.button_l;
                        break;
                    
                    case TRIG_MODE_A2D:
                        ns_input.trigger_zl = (adj_tl >= adapter_settings.trigger_threshold_l) ? 1 : 0;
                        break;
                }

                switch( adapter_settings.ns_trigger_r)
                {
                    default:
                    case TRIG_MODE_OFF:
                        ns_input.trigger_zr = gc_poll_response.button_r;
                        break;
                    
                    case TRIG_MODE_A2D:
                        ns_input.trigger_zr = (adj_tr >= adapter_settings.trigger_threshold_r) ? 1 : 0;
                        break;
                }
                
                adj_x   = (int) gc_poll_response.stick_x - gc_origin_data.stick_x;
                adj_y   = 256 - ( (int) gc_poll_response.stick_y - gc_origin_data.stick_y );
                adj_cx  = (int) gc_poll_response.cstick_x - gc_origin_data.cstick_x;
                adj_cy  = 256 - ( (int) gc_poll_response.cstick_y - gc_origin_data.cstick_y ); 

                ns_input.stick_left_x   = scale_axis(adj_x);
                ns_input.stick_left_y   = scale_axis(adj_y);
                ns_input.stick_right_x  = scale_axis(adj_cx);
                ns_input.stick_right_y  = scale_axis(adj_cy);
                
            }
            
            memcpy(&ns_buffer, &ns_input, NS_HID_LEN);
            // Send USB report
            if (usb_clear && tud_hid_ready())
            {
                usb_clear = false;
                tud_hid_report(0, &ns_buffer, NS_HID_LEN);
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

                adj_tl  = (int) gc_poll_response.trigger_l  - gc_origin_data.trigger_l;
                adj_tr  = (int) gc_poll_response.trigger_r  - gc_origin_data.trigger_r;

                switch( adapter_settings.gc_trigger_l)
                {
                    default:
                    case TRIG_MODE_OFF:
                        gc_input.trigger_l          = scale_trigger(adj_tl);
                        gc_input.button_l           = gc_poll_response.button_l;
                        break;
                    
                    case TRIG_MODE_A2D:
                        gc_input.trigger_l          = (adj_tl >= adapter_settings.trigger_threshold_l) ? 255 : scale_trigger(adj_tl);
                        gc_input.button_l           = (adj_tl >= adapter_settings.trigger_threshold_l) ? 1 : 0;
                        break;

                    case TRIG_MODE_D2ALITE:
                        gc_input.trigger_l          = (gc_poll_response.button_l) ? 85 : 0;
                        gc_input.button_l           = 0;
                        break;
                    
                    case TRIG_MODE_D2AFULL:
                        gc_input.trigger_l          = (gc_poll_response.button_l) ? 255 : 0;
                        gc_input.button_l           = gc_poll_response.button_l;
                        break;
                }

                switch( adapter_settings.gc_trigger_r)
                {
                    default:
                    case TRIG_MODE_OFF:
                        gc_input.trigger_r      = scale_trigger(adj_tr);
                        gc_input.button_r       = gc_poll_response.button_r;
                        break;
                    
                    case TRIG_MODE_A2D:
                        gc_input.trigger_r      = (adj_tr >= adapter_settings.trigger_threshold_r) ? 255 : scale_trigger(adj_tr);
                        gc_input.button_r       = (adj_tr >= adapter_settings.trigger_threshold_r) ? 1 : 0;
                        break;

                    case TRIG_MODE_D2ALITE:
                        gc_input.trigger_r          = (gc_poll_response.button_r) ? 85 : 0;
                        gc_input.button_r           = 0;
                        break;
                    
                    case TRIG_MODE_D2AFULL:
                        gc_input.trigger_r          = (gc_poll_response.button_r) ? 255 : 0;
                        gc_input.button_r           = gc_poll_response.button_r;
                        break;
                }

                adj_x   = (int) gc_poll_response.stick_x    - gc_origin_data.stick_x;
                adj_y   = (int) gc_poll_response.stick_y    - gc_origin_data.stick_y;
                adj_cx  = (int) gc_poll_response.cstick_x   - gc_origin_data.cstick_x;
                adj_cy  = (int) gc_poll_response.cstick_y   - gc_origin_data.cstick_y;

                gc_input.stick_x        = (uint8_t) adj_x;
                gc_input.stick_y        = (uint8_t) adj_y;
                gc_input.cstick_x       = (uint8_t) adj_cx;
                gc_input.cstick_y       = (uint8_t) adj_cy;
                
            }

            if (first)
            {
                memcpy(&gc_buffer[2], &gc_input, 8);

                /*GC adapter notes for new data
                
                with only black USB plugged in
                - no controller, byte 1 is 0
                - controller plugged in to port 1, byte 1 is 0x10
                - controller plugged in port 2, byte 10 is 0x10

                with both USB plugged in
                - no controller, byte 1 is 0x04
                - controller plugged in to port 1, byte is 0x14 */
                gc_buffer[1] = 0x14;
                gc_buffer[10] = 0x04;
                gc_buffer[19] = 0x04;
                gc_buffer[28] = 0x04;
            }
            
            gc_buffer[0] = 0x21;

            if (usb_clear && tud_hid_ready())
            {
                usb_clear = false;
                if (tud_hid_report(0, &gc_buffer, GC_HID_LEN))
                {
                    first = true;
                }
                else
                {
                    ESP_LOGE("USB", "Failed send");
                }
            }
            
            break;

        case USB_MODE_XINPUT:
            xi_input.report_id = 0x01;
            if (!repeat)
            {

                if (gc_poll_response.button_start && gc_poll_response.button_a)
                {
                    xi_input.button_menu = 1;
                }
                else if (gc_poll_response.button_start && gc_poll_response.button_b)
                {
                    xi_input.button_back = 1;
                }
                else
                {
                    xi_input.button_a = gc_poll_response.button_a;
                    xi_input.button_b = gc_poll_response.button_b;
                    xi_input.button_x = gc_poll_response.button_x;
                    xi_input.button_y = gc_poll_response.button_y;
                    if (gc_poll_response.button_start && gc_poll_response.button_z)
                    {
                        xi_input.bumper_l = 1;
                    }
                    else
                    {
                        xi_input.bumper_r = gc_poll_response.button_z;
                        xi_input.bumper_l = 0;
                    }
                    

                    xi_input.button_menu   = 0;
                    xi_input.button_back    = 0;
                }

                adj_x   = (int) gc_poll_response.stick_x         - gc_origin_data.stick_x;
                adj_y   = 256 - ((int) gc_poll_response.stick_y  - gc_origin_data.stick_y);
                adj_cx  = (int) gc_poll_response.cstick_x        - gc_origin_data.cstick_x;
                adj_cy  = 256 - ((int) gc_poll_response.cstick_y - gc_origin_data.cstick_y);

                adj_tl  = (int) gc_poll_response.trigger_l       - gc_origin_data.trigger_l;
                adj_tr  = (int) gc_poll_response.trigger_r       - gc_origin_data.trigger_r;

                switch( adapter_settings.xi_trigger_l)
                {
                    default:
                    case TRIG_MODE_OFF:
                        xi_input.analog_trigger_l = (uint16_t) scale_trigger(adj_tl)<<2;
                        break;
                    
                    case TRIG_MODE_A2D:
                        xi_input.analog_trigger_l   = (adj_tl >= adapter_settings.trigger_threshold_l) ? (255<<2) : (uint16_t) scale_trigger(adj_tl)<<2;
                        break;
                }

                switch( adapter_settings.xi_trigger_r)
                {
                    default:
                    case TRIG_MODE_OFF:
                        xi_input.analog_trigger_r = (uint16_t) scale_trigger(adj_tr)<<2;
                        break;
                    
                    case TRIG_MODE_A2D:
                        xi_input.analog_trigger_r   = (adj_tr >= adapter_settings.trigger_threshold_r) ? (255<<2) : (uint16_t) scale_trigger(adj_tr)<<2;
                        break;
                }

                xi_input.stick_left_x   = (uint16_t) scale_axis(adj_x)  << 8;
                xi_input.stick_left_y   = (uint16_t) scale_axis(adj_y)  << 8;
                xi_input.stick_right_x  = (uint16_t) scale_axis(adj_cx) << 8;
                xi_input.stick_right_y  = (uint16_t) scale_axis(adj_cy) << 8;

                uint8_t lr = 1 - gc_poll_response.dpad_left + gc_poll_response.dpad_right;
                uint8_t ud = 1 - gc_poll_response.dpad_down + gc_poll_response.dpad_up;

                xi_input.dpad_hat = dir_to_hat(HAT_MODE_XI, lr, ud);
            }
            
            if (usb_clear && tud_ready())
            {
                memcpy(xi_buffer, &xi_input, sizeof(xi_input_s));
                usb_clear = false;
                tud_hid_report(0, xi_buffer, XI_HID_LEN);
            }
            
            break;

    }
}