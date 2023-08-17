#include "swpro.h"

/** Switch PRO HID MODE **/
// 1. Device Descriptor
// 2. HID Report Descriptor
// 3. Configuration Descriptor
// 4. TinyUSB Config
/**--------------------------**/
const tusb_desc_device_t swpro_device_descriptor = {
    .bLength = 18,
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0210, // Changed from 0x0200 to 2.1 for BOS & WebUSB
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x057E,
    .idProduct = 0x2009,

    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01};

const uint8_t swpro_hid_report_descriptor[] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x15, 0x00, // Logical Minimum (0)

    0x09, 0x04, // Usage (Joystick)
    0xA1, 0x01, // Collection (Application)

    0x85, 0x30, //   Report ID (48)
    0x05, 0x01, //   Usage Page (Generic Desktop Ctrls)
    0x05, 0x09, //   Usage Page (Button)
    0x19, 0x01, //   Usage Minimum (0x01)
    0x29, 0x0A, //   Usage Maximum (0x0A)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x0A, //   Report Count (10)
    0x55, 0x00, //   Unit Exponent (0)
    0x65, 0x00, //   Unit (None)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09, //   Usage Page (Button)
    0x19, 0x0B, //   Usage Minimum (0x0B)
    0x29, 0x0E, //   Usage Maximum (0x0E)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x04, //   Report Count (4)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x02, //   Report Count (2)
    0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x0B, 0x01, 0x00, 0x01, 0x00, //   Usage (0x010001)
    0xA1, 0x00,                   //   Collection (Physical)
    0x0B, 0x30, 0x00, 0x01, 0x00, //     Usage (0x010030)
    0x0B, 0x31, 0x00, 0x01, 0x00, //     Usage (0x010031)
    0x0B, 0x32, 0x00, 0x01, 0x00, //     Usage (0x010032)
    0x0B, 0x35, 0x00, 0x01, 0x00, //     Usage (0x010035)
    0x15, 0x00,                   //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00, //     Logical Maximum (65534)
    0x75, 0x10,                   //     Report Size (16)
    0x95, 0x04,                   //     Report Count (4)
    0x81, 0x02,                   //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,                         //   End Collection

    0x0B, 0x39, 0x00, 0x01, 0x00, //   Usage (0x010039)
    0x15, 0x00,                   //   Logical Minimum (0)
    0x25, 0x07,                   //   Logical Maximum (7)
    0x35, 0x00,                   //   Physical Minimum (0)
    0x46, 0x3B, 0x01,             //   Physical Maximum (315)
    0x65, 0x14,                   //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,                   //   Report Size (4)
    0x95, 0x01,                   //   Report Count (1)
    0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,                   //   Usage Page (Button)
    0x19, 0x0F,                   //   Usage Minimum (0x0F)
    0x29, 0x12,                   //   Usage Maximum (0x12)
    0x15, 0x00,                   //   Logical Minimum (0)
    0x25, 0x01,                   //   Logical Maximum (1)
    0x75, 0x01,                   //   Report Size (1)
    0x95, 0x04,                   //   Report Count (4)
    0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,                   //   Report Size (8)
    0x95, 0x34,                   //   Report Count (52)
    0x81, 0x03,                   //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined 0xFF00)
    0x85, 0x21,       //   Report ID (33)
    0x09, 0x01,       //   Usage (0x01)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x3F,       //   Report Count (63)
    0x81, 0x03,       //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x81, //   Report ID (-127)
    0x09, 0x02, //   Usage (0x02)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x01, //   Report ID (1)
    0x09, 0x03, //   Usage (0x03)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x10, //   Report ID (16)
    0x09, 0x04, //   Usage (0x04)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x80, //   Report ID (-128)
    0x09, 0x05, //   Usage (0x05)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x82, //   Report ID (-126)
    0x09, 0x06, //   Usage (0x06)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0xC0, // End Collection

    // 203 bytes
};

const uint8_t swpro_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, 64, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // Interface
    9,
    TUSB_DESC_INTERFACE,
    0x00,
    0x00,
    0x02,
    TUSB_CLASS_HID,
    0x00,
    0x00,
    0x00,
    // HID Descriptor
    9,
    HID_DESC_TYPE_HID,
    U16_TO_U8S_LE(0x0111),
    0,
    1,
    HID_DESC_TYPE_REPORT,
    U16_TO_U8S_LE(sizeof(swpro_hid_report_descriptor)),
    // Endpoint Descriptor
    7,
    TUSB_DESC_ENDPOINT,
    0x81,
    TUSB_XFER_INTERRUPT,
    U16_TO_U8S_LE(64),
    8,
    // Endpoint Descriptor
    7,
    TUSB_DESC_ENDPOINT,
    0x01,
    TUSB_XFER_INTERRUPT,
    U16_TO_U8S_LE(64),
    8,

    // Alternate Interface for WebUSB
    // Interface
    9,
    TUSB_DESC_INTERFACE,
    0x01,
    0x00,
    0x02,
    TUSB_CLASS_VENDOR_SPECIFIC,
    0x00,
    0x00,
    0x00,
    // Endpoint Descriptor
    7,
    TUSB_DESC_ENDPOINT,
    0x82,
    TUSB_XFER_BULK,
    U16_TO_U8S_LE(64),
    0,
    // Endpoint Descriptor
    7,
    TUSB_DESC_ENDPOINT,
    0x02,
    TUSB_XFER_BULK,
    U16_TO_U8S_LE(64),
    0,
};

const tinyusb_config_t swpro_cfg = {
    .device_descriptor = &swpro_device_descriptor,
    .string_descriptor = global_string_descriptor,
    .external_phy = false,
    .configuration_descriptor = swpro_configuration_descriptor,
};

/**--------------------------**/
/**--------------------------**/

bool mac_sent = false;
bool blank_sent = false;

uint32_t _timeout = 0;

void swpro_hid_report(gc_poll_response_s *poll_response, gc_origin_data_s *origin_data)
{
    static sw_input_s data = {0};

    data.d_down = poll_response->dpad_down;
    data.d_right = poll_response->dpad_right;
    data.d_left = poll_response->dpad_left;
    data.d_up = poll_response->dpad_up;

    data.b_y = poll_response->button_y;
    data.b_x = poll_response->button_x;
    data.b_a = poll_response->button_a;
    data.b_b = poll_response->button_b;

    // data.b_minus  = poll_response->button_minus;
    data.b_plus = poll_response->button_start;
    // data.b_home   = poll_response->button_home;
    // data.b_capture = poll_response->button_capture;

    // data.sb_right = poll_response->button_stick_right;
    // data.sb_left  = poll_response->button_stick_left;

    data.t_r = poll_response->button_z;
    // data.t_l  = poll_response->trigger_l;

    int adj_x = gc_origin_adjust(poll_response->stick_x, origin_data->stick_x, false);
    int adj_y = gc_origin_adjust(poll_response->stick_y, origin_data->stick_y, false);
    int adj_cx = gc_origin_adjust(poll_response->cstick_x, origin_data->cstick_x, false);
    int adj_cy = gc_origin_adjust(poll_response->cstick_y, origin_data->cstick_y, false);

    adj_x = scale_axis(adj_x);
    adj_y = scale_axis(adj_y);
    adj_cx = scale_axis(adj_cx);
    adj_cy = scale_axis(adj_cy);
    data.t_zl = poll_response->trigger_l > 75 ? 1 : 0;
    data.t_zr = poll_response->trigger_r > 75 ? 1 : 0;

    data.ls_x = adj_x << 4;
    data.ls_y = adj_y << 4;
    data.rs_x = adj_cx << 4;
    data.rs_y = adj_cy << 4;

    switch_commands_process(&data);
}
