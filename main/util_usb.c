#include "util_usb.h"

usb_mode_t adapter_mode         = USB_MODE_NS;
uint16_t usb_timeout_time       = 0;

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
    .iSerialNumber = 0x00,
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
            0x09, 0x36,        //   Usage (Slider)
            0x09, 0x37,        //   Usage (Dial)

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
            0x95, 0x0A,            //            REPORT_COUNT (10) 
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xc0,

        0xA1, 0x01,         // Collection (Application)
            0x06, 0x00, 0xFF,      //            USAGE_PAGE (Vendor Defined Page 1) 
            0x09, 0x01,            //            USAGE (Vendor Usage 1) 
            0x85, 0x02,            //           Report ID (2)
            0x15, 0x00,            //            LOGICAL_MINIMUM (0) 
            0x26, 0xff, 0x00,       //            LOGICAL_MAXIMUM (255) 
            0x75, 0x08,            //            REPORT_SIZE (8) 
            0x95, 0x0A,            //            REPORT_COUNT (10) 
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
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(64), 1,
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
    .iSerialNumber = 0x00,
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
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(ns_hid_report_descriptor), 0x81, 64, 1),
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


void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
    switch (active_usb_mode)
    {
        case USB_MODE_GENERIC:
            if ((report[0] == 0x01) || (report[0] == 0x02))
            {
                usb_process_data();
            }
            break;

        default:
        case USB_MODE_NS:
            if (len == NS_HID_LEN)
            {
                usb_process_data();
            }
            break;
        case USB_MODE_XINPUT:
            if ( (report[0] == 0x00) && (report[1] == XID_REPORT_LEN))
            {
                usb_process_data();
            }

            break;

        case USB_MODE_GC:
            if (report[0] == 0x21)
            {
                usb_process_data();
            }
            break;
    }
    
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{

    switch (active_usb_mode)
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
                    rx_vibrate = (buffer[1] > 0) ? true : false;
                }
                else if (buffer[0] == 0x13)
                {
                    ESP_LOGI("INIT", "Rx");
                }
            }
            break;
        case USB_MODE_XINPUT:
            if (!report_id && !report_type)
            {
                if ((buffer[0] == 0x00) && (buffer[1] == 0x08))
                {
                    if ((buffer[3] > 0) || (buffer[4] > 0))
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
    switch (active_usb_mode)
    {
        default:
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

// Sets up custom TinyUSB Device Driver
usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
    ESP_LOGI("USB DRIVER CB", "Installing XInput Driver");
    *driver_count += 1;
    return &tud_xinput_driver;
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

uint8_t scale_axis(int input)
{
    int res = input;

    if (input > 255)
    {
        input = 255;
    }
    if (input < 0)
    {
        input = 0;
    }

    if (input > 127)
    {
        float tmp = (float) input - 127;
        tmp = tmp * analog_scaler_f;
        res = (int) tmp + 127;
    }
    else if (input < 127)
    {
        float tmp = 127 - (float) input;
        tmp = tmp * analog_scaler_f;
        res = 127 - (int) tmp;    
    }
    else
    {
        res = 127;
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

#define SIGNED_SCALER (float) 
short sign_axis(int input)
{
    uint8_t scaled = scale_axis(input);

    int start = (int) scaled - 127;
    if ((start * 256) > 32765)
    {
        start = 32765;
    }
    else if ((start * 256) < -32765)
    {
        start = -32765;
    }
    else
    {
        start *= 256;
    }
    return (short) start;
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

int gc_origin_adjust(uint8_t value, int origin, bool invert)
{
    int out = 0;

    if(invert)
    {
        out = 255 - ((int) value - origin);
    }
    else
    {
        out = (int) value - origin;
    }

    if (out < 0)
    {
        out = 0;
    }
    else if (out > 255)
    {
        out = 255;
    }

    return out;
    
}

void gcusb_start(usb_mode_t mode)
{
    const char* TAG = "gcusb_start";

    ESP_LOGI(TAG, "USB initialization");

    switch (mode)
    {
        default:
        case USB_MODE_NS:
            ESP_LOGI(TAG, "NS MODE");
            ESP_ERROR_CHECK(tinyusb_driver_install(&ns_cfg));
            break;
        case USB_MODE_GENERIC:
            ESP_LOGI(TAG, "DINPUT MODE");
            ESP_ERROR_CHECK(tinyusb_driver_install(&di_cfg));
            break;
        case USB_MODE_GC:
            ESP_LOGI(TAG, "GCC MODE");
            ESP_ERROR_CHECK(tinyusb_driver_install(&gc_cfg));
            break;

        case USB_MODE_XINPUT:
            ESP_LOGI(TAG, "LEGACY XINPUT MODE");
            ESP_ERROR_CHECK(xinput_driver_install());
            break;
    }

    while(!tud_mounted())
    {
        vTaskDelay(8/portTICK_PERIOD_MS);
    }
    if (mode != USB_MODE_XINPUT)
    {
        while (!tud_hid_ready())
        {
            vTaskDelay(8/portTICK_PERIOD_MS);
        }
    }
    else
    {
        while(!tud_xinput_ready())
            {
                vTaskDelay(8/portTICK_PERIOD_MS);
            }
    }
    vTaskDelay(250/portTICK_PERIOD_MS);
    usb_send_data();
}

int adj_x;
int adj_y;
int adj_cx;
int adj_cy;
int adj_tl;
int adj_tr;

void dinput_send_data(void)
{
    di_input.report_id = 0x01;

    if (cmd_phase != CMD_PHASE_POLL)
    {
        di_input.buttons_1 = 0x00;
        di_input.buttons_2 = 0x00;
        di_input.dpad_hat = NS_HAT_CENTER;
        di_input.stick_left_x = 127;
        di_input.stick_left_y = 127;
        di_input.stick_right_x = 127;
        di_input.stick_right_y = 127;
        di_input.analog_trigger_l = 0;
        di_input.analog_trigger_r = 0;
    }
    else
    {
        // Generate the USB Data for NS mode
        di_input.button_a = gc_poll_response.button_a;
        di_input.button_b = gc_poll_response.button_b;
        
        // Defaults
        di_input.button_y       = gc_poll_response.button_y;
        di_input.button_x       = gc_poll_response.button_x;
        di_input.trigger_r      = gc_poll_response.button_z;

        if (adapter_settings.di_zjump == 1)
        {
            di_input.button_x       = gc_poll_response.button_z;
            di_input.trigger_r       = gc_poll_response.button_x;
        }
        else if (adapter_settings.di_zjump == 2)
        {
            di_input.button_y       = gc_poll_response.button_z;
            di_input.trigger_r       = gc_poll_response.button_y;
        }

        uint32_t regread = REG_READ(GPIO_IN_REG) & PIN_MASK_GCP;
        di_input.button_home = !util_getbit(regread, NEXT_BUTTON);
        di_input.button_capture = !util_getbit(regread, PREV_BUTTON);

        uint8_t lr = 1 - gc_poll_response.dpad_left + gc_poll_response.dpad_right;
        uint8_t ud = 1 - gc_poll_response.dpad_down + gc_poll_response.dpad_up;

        di_input.dpad_hat = dir_to_hat(HAT_MODE_NS, lr, ud);

        di_input.button_plus = gc_poll_response.button_start;

        // Get origin modified Trigger analog data
        adj_tl  = gc_origin_adjust(gc_poll_response.trigger_l, gc_origin_data.trigger_l,  false);
        adj_tr  = gc_origin_adjust(gc_poll_response.trigger_r, gc_origin_data.trigger_r,  false);

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
        
        adj_x   = gc_origin_adjust(gc_poll_response.stick_x,  gc_origin_data.stick_x,     false);
        adj_y   = gc_origin_adjust(gc_poll_response.stick_y,  gc_origin_data.stick_y,     true);
        adj_cx  = gc_origin_adjust(gc_poll_response.cstick_x, gc_origin_data.cstick_x,    false);
        adj_cy  = gc_origin_adjust(gc_poll_response.cstick_y, gc_origin_data.cstick_y,    false);

        di_input.stick_left_x   = (uint8_t) adj_x;
        di_input.stick_left_y   = (uint8_t) adj_y;
        di_input.stick_right_x  = (uint8_t) adj_cx;
        di_input.stick_right_y  = (uint8_t) adj_cy;
    }
    
    tud_hid_report(0, &di_input, DI_HID_LEN);
}

void xinput_send_data(void)
{
    xid_input.report_id = 0x00;
    xid_input.report_size = 20;

    if (cmd_phase != CMD_PHASE_POLL)
    {
        xid_input.buttons_1 = 0x00;
        xid_input.buttons_2 = 0x00;
        xid_input.stick_left_x = 0;
        xid_input.stick_left_y = 0;
        xid_input.stick_right_x = 0;
        xid_input.stick_right_y = 0;
        xid_input.analog_trigger_l = 0;
        xid_input.analog_trigger_r = 0;
    }
    else
    {

        uint32_t regread = REG_READ(GPIO_IN_REG) & PIN_MASK_GCP;
        xid_input.button_guide = !util_getbit(regread, NEXT_BUTTON);
        xid_input.button_back =  !util_getbit(regread, PREV_BUTTON);

        xid_input.button_a = gc_poll_response.button_a;
        xid_input.button_b = gc_poll_response.button_b;

        xid_input.dpad_left     = gc_poll_response.dpad_left;
        xid_input.dpad_right    = gc_poll_response.dpad_right;
        xid_input.dpad_down     = gc_poll_response.dpad_down;
        xid_input.dpad_up       = gc_poll_response.dpad_up;

        //Defaults
        xid_input.button_y       = gc_poll_response.button_y;
        xid_input.button_x       = gc_poll_response.button_x;
        xid_input.bumper_r       = gc_poll_response.button_z;
        xid_input.bumper_l       = 0;
        xid_input.button_menu    = 0;

        if (adapter_settings.xi_zjump == 1)
        {
            xid_input.button_x       = gc_poll_response.button_z;
            xid_input.bumper_r       = gc_poll_response.button_x;
        }
        else if (adapter_settings.xi_zjump == 2)
        {
            xid_input.button_y       = gc_poll_response.button_z;
            xid_input.bumper_r       = gc_poll_response.button_y;
        }

        if (gc_poll_response.button_start && gc_poll_response.button_z)
        {
            xid_input.bumper_l = 1;
            xid_input.bumper_r = 0;
            xid_input.button_x = 0;
        }
        else if (gc_poll_response.button_start)
        {
            xid_input.button_menu = 1;
        }

        adj_x   = gc_origin_adjust(gc_poll_response.stick_x,  gc_origin_data.stick_x,     false);
        adj_y   = gc_origin_adjust(gc_poll_response.stick_y,  gc_origin_data.stick_y,     true);
        adj_cx  = gc_origin_adjust(gc_poll_response.cstick_x, gc_origin_data.cstick_x,    false);
        adj_cy  = gc_origin_adjust(gc_poll_response.cstick_y, gc_origin_data.cstick_y,    true);

        adj_tl  = gc_origin_adjust(gc_poll_response.trigger_l, gc_origin_data.trigger_l,  false);
        adj_tr  = gc_origin_adjust(gc_poll_response.trigger_r, gc_origin_data.trigger_r,  false);

        switch( adapter_settings.xi_trigger_l)
        {
            default:
            case TRIG_MODE_OFF:
                xid_input.analog_trigger_l = scale_trigger(adj_tl);
                break;
            
            case TRIG_MODE_A2D:
                xid_input.analog_trigger_l   = gc_poll_response.button_l * 255;
                break;
        }

        switch( adapter_settings.xi_trigger_r)
        {
            default:
            case TRIG_MODE_OFF:
                xid_input.analog_trigger_r = scale_trigger(adj_tr);
                break;
            
            case TRIG_MODE_A2D:
                xid_input.analog_trigger_r   = gc_poll_response.button_r * 255;
                break;
        }

        xid_input.stick_left_x   = sign_axis(adj_x);
        xid_input.stick_left_y   = sign_axis(adj_y)*-1;
        xid_input.stick_right_x  = sign_axis(adj_cx);
        xid_input.stick_right_y  = sign_axis(adj_cy)*-1;
    }

    tud_xinput_report(&xid_input, XID_REPORT_LEN);

}

bool gc_first = false;
void gc_send_data(void)
{
    gc_buffer[0] = 0x21;

    if (cmd_phase != CMD_PHASE_POLL)
    {
        gc_input.buttons_1 = 0x00;
        gc_input.buttons_2 = 0x00;
        gc_input.stick_x    = 127;
        gc_input.stick_y    = 127;
        gc_input.cstick_x   = 127;
        gc_input.cstick_y   = 127;
        gc_input.trigger_l = 0;
        gc_input.trigger_r = 0;
    }
    else
    {
        // Generate the USB Data for GameCube native mode
        gc_input.button_a   = gc_poll_response.button_a;
        gc_input.button_b   = gc_poll_response.button_b;

        gc_input.dpad_down  = gc_poll_response.dpad_down;
        gc_input.dpad_up    = gc_poll_response.dpad_up;
        gc_input.dpad_left  = gc_poll_response.dpad_left;
        gc_input.dpad_right = gc_poll_response.dpad_right;

        gc_input.button_start   = gc_poll_response.button_start;

        // Defaults
        gc_input.button_y       = gc_poll_response.button_y;
        gc_input.button_x       = gc_poll_response.button_x;
        gc_input.button_z       = gc_poll_response.button_z;

        if (adapter_settings.gc_zjump == 1)
        {
            gc_input.button_x       = gc_poll_response.button_z;
            gc_input.button_z       = gc_poll_response.button_x;
        }
        else if (adapter_settings.gc_zjump == 2)
        {
            gc_input.button_y       = gc_poll_response.button_z;
            gc_input.button_z       = gc_poll_response.button_y;
        }

        adj_tl  = gc_origin_adjust(gc_poll_response.trigger_l, gc_origin_data.trigger_l,  false);
        adj_tr  = gc_origin_adjust(gc_poll_response.trigger_r, gc_origin_data.trigger_r,  false);

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

        adj_x   = gc_origin_adjust(gc_poll_response.stick_x,  gc_origin_data.stick_x,     false);
        adj_y   = gc_origin_adjust(gc_poll_response.stick_y,  gc_origin_data.stick_y,     false);
        adj_cx  = gc_origin_adjust(gc_poll_response.cstick_x, gc_origin_data.cstick_x,    false);
        adj_cy  = gc_origin_adjust(gc_poll_response.cstick_y, gc_origin_data.cstick_y,    false);

        gc_input.stick_x        = (uint8_t) adj_x;
        gc_input.stick_y        = (uint8_t) adj_y;
        gc_input.cstick_x       = (uint8_t) adj_cx;
        gc_input.cstick_y       = (uint8_t) adj_cy;
    }
    

    if (!gc_first)
    {
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
        gc_first = true;
    }
    else
    {
        memcpy(&gc_buffer[2], &gc_input, 8);
    }

    tud_hid_report(0, &gc_buffer, GC_HID_LEN);
    
}


void ns_send_data(void)
{
    if (cmd_phase != CMD_PHASE_POLL)
    {
        ns_input.buttons_1      = 0x00;
        ns_input.buttons_2      = 0x00;
        ns_input.dpad_hat       = NS_HAT_CENTER;
        ns_input.stick_left_x   = 127;
        ns_input.stick_left_y   = 127;
        ns_input.stick_right_x  = 127;
        ns_input.stick_left_y   = 127;
    }
    else
    {

        ns_input.button_a = gc_poll_response.button_a;
        ns_input.button_b = gc_poll_response.button_b;

        // Defaults
        ns_input.button_y       = gc_poll_response.button_y;
        ns_input.button_x       = gc_poll_response.button_x;
        ns_input.trigger_r      = gc_poll_response.button_z;

        if (adapter_settings.ns_zjump == 1)
        {
            ns_input.button_x       = gc_poll_response.button_z;
            ns_input.trigger_r       = gc_poll_response.button_x;
        }
        else if (adapter_settings.ns_zjump == 2)
        {
            ns_input.button_y       = gc_poll_response.button_z;
            ns_input.trigger_r       = gc_poll_response.button_y;
        }

        uint8_t lr = 1 - gc_poll_response.dpad_left + gc_poll_response.dpad_right;
        uint8_t ud = 1 - gc_poll_response.dpad_down + gc_poll_response.dpad_up;

        ns_input.dpad_hat = dir_to_hat(HAT_MODE_NS, lr, ud);

        uint32_t regread = REG_READ(GPIO_IN_REG) & PIN_MASK_GCP;
        ns_input.button_home = !util_getbit(regread, NEXT_BUTTON);
        ns_input.button_capture = !util_getbit(regread, PREV_BUTTON);

        ns_input.button_plus = gc_poll_response.button_start;

        adj_tl  = gc_origin_adjust(gc_poll_response.trigger_l, gc_origin_data.trigger_l,  false);
        adj_tr  = gc_origin_adjust(gc_poll_response.trigger_r, gc_origin_data.trigger_r,  false);

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

        adj_x   = gc_origin_adjust(gc_poll_response.stick_x,  gc_origin_data.stick_x,     false);
        adj_y   = gc_origin_adjust(gc_poll_response.stick_y,  gc_origin_data.stick_y,     true);
        adj_cx  = gc_origin_adjust(gc_poll_response.cstick_x, gc_origin_data.cstick_x,    false);
        adj_cy  = gc_origin_adjust(gc_poll_response.cstick_y, gc_origin_data.cstick_y,    true);

        ns_input.stick_left_x   = scale_axis(adj_x);
        ns_input.stick_left_y   = scale_axis(adj_y);
        ns_input.stick_right_x  = scale_axis(adj_cx);
        ns_input.stick_right_y  = scale_axis(adj_cy);
    }

    tud_hid_report(0, &ns_input, NS_HID_LEN);
}


void usb_send_data(void)
{
    if (!tud_ready())
    {
        return;
    }
    // Send USB data according to the adapter mode
    switch (active_usb_mode)
    {
        default:
        case USB_MODE_GENERIC:
            dinput_send_data();
            break;
        case USB_MODE_NS:
            ns_send_data();
            break;
        case USB_MODE_GC:
            gc_send_data();
            break;
        case USB_MODE_XINPUT:
            xinput_send_data();
            break;
    }
}

// Some definitions for USB Timing
#define TIME_USB_US 22
#define TIME_GC_POLL 410/2
#define TIMEOUT_GC_US 500
#define TIMEOUT_COUNTS 10

#define TIME_ENDCAP_MAX 550

// The philosophy behind dynamic HID polling alignment
/* 
You have T1, which is the timestamp on which the RMT tx is started
You have T2, which is the time it took for the USB packet to send

We want to calculate the exact center of the minimum polling cycle
in a given scenario.
*/

// This is our time counter that we can use
// for calculations
uint64_t usb_delay_time = 0;

// This is the calculated delay we add
// We only add this when we enter POLLING
uint64_t usb_time_offset = 50;

uint64_t rmt_poll_time = 0;

void rmt_reset()
{
    JB_RX_MEMOWNER  = 1;
    JB_RX_RDRST     = 1;
    JB_RX_RDRST     = 0;
    JB_RX_CLEARISR  = 1;
    JB_RX_BEGIN     = 0;
    JB_RX_SYNC      = 1;
    JB_RX_SYNC      = 0;
    JB_RX_BEGIN     = 1;
    JB_TX_RDRST     = 1;
    JB_TX_WRRST     = 1;
    JB_TX_CLEARISR  = 1;
}

// This is called after each successful USB report send.
void usb_process_data(void)
{
    usb_timeout_time = 0;
    // Check if we have config data to send out
    if(cmd_flagged)
    {
        gc_timer_stop();
        gc_timer_reset();
        command_queue_process();
        rmt_reset();
        return;
    }
    
    if (cmd_phase == CMD_PHASE_POLL)
    {
        if (active_gc_type == GC_TYPE_WIRED)
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = (rx_vibrate == true) ? JB_HIGH : JB_LOW;
        }
        else if (active_gc_type == GC_TYPE_WAVEBIRD)
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = JB_LOW;
        }

        if (gc_timer_status == GC_TIMER_IDLE)
        {
            gc_timer_start();
        }
        else if (gc_timer_status == GC_TIMER_STARTED)
        {
            gptimer_get_raw_count(gc_timer, &usb_delay_time);
            gc_timer_reset();

            // Calculate new time delay that we use during polling for
            // perfectly centered polls (Only valid for above 2ms refresh)
            if (usb_delay_time >= 2500)
            {
                usb_time_offset = (usb_delay_time/2) - TIME_GC_POLL - TIME_USB_US;
            }
            // Otherwise we know we're polling at 1ms and where we need to place the poll
            else
            {   
                usb_time_offset = 500-TIME_GC_POLL;
            }

            ets_delay_us(usb_time_offset);
        }
    }
    else
    {
        ets_delay_us(50);
    }
    
    // Start RMT transaction
    // Set mem owner
    JB_RX_MEMOWNER  = 1;
    // Set RX to begin so it starts when sync bit is set.
    JB_RX_BEGIN     = 1;
    // Start next transaction.
    JB_TX_BEGIN     = 1;
    
    ets_delay_us(TIMEOUT_GC_US);

    // If we timed out, just reset for next phase
    if (!rx_recieved)
    {
        
        rmt_reset();

        rx_timeout_counts += 1;
        if (rx_timeout_counts >= TIMEOUT_COUNTS)
        {
            gc_timer_stop();
            gc_timer_reset();

            rx_timeout_counts = 0;
            cmd_phase = CMD_PHASE_PROBE;
            rgb_animate_to(COLOR_RED);

            memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GCMD_PROBE_LEN);
        }
    }
    else if (rx_recieved)
    {
        rx_recieved = false;
        rx_timeout_counts = 0;
        // Process our data if we received something
        // from the gamecube controller
        gamecube_rmt_process();
        rmt_reset();
    }

    usb_send_data();
}