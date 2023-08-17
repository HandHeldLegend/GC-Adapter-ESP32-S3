#include "gamecube.h"

/** GAMECUBE HID MODE **/
// 1. Device Descriptor
// 2. HID Report Descriptor
// 3. Configuration Descriptor
// 4. TinyUSB Config
/**--------------------------**/

/**** GameCube Adapter Device Descriptor ****/
const tusb_desc_device_t gc_descriptor_dev = {
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
const uint8_t gc_hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, 41, TUSB_DESC_CONFIG_ATT_SELF_POWERED, 500),
    // Interface
    9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_HID, 0x00, 0x00, 0x00,
    // HID Descriptor
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(sizeof(gc_hid_report_descriptor)),
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x81, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(37), 8,
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(6), 8,
};

const uint8_t gc_hid_configuration_descriptor_performance[] = {
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
const tinyusb_config_t gc_cfg = {
    .device_descriptor          = &gc_descriptor_dev,
    .string_descriptor          = global_string_descriptor,
    .external_phy               = false,
    .configuration_descriptor   = gc_hid_configuration_descriptor,
};

const tinyusb_config_t gc_cfg_perf = {
    .device_descriptor          = &gc_descriptor_dev,
    .string_descriptor          = global_string_descriptor,
    .external_phy               = false,
    .configuration_descriptor   = gc_hid_configuration_descriptor_performance,
};

static bool gc_first = false;
static uint8_t gc_buffer[GC_HID_LEN] = {0};
static gc_input_s gc_input = {0};

void gamecube_reset_data()
{
    memset(gc_buffer, 0, GC_HID_LEN);
    gc_first = false;
}

void gamecube_hid_report(gc_poll_response_s *poll_response, gc_origin_data_s *origin_data)
{
    gc_buffer[0] = 0x21;

    if (cmd_phase != CMD_PHASE_POLL)
    {
        gc_input.buttons_1 = 0x00;
        gc_input.buttons_2 = 0x00;
        gc_input.stick_x    = GC_AXIS_CENTER;
        gc_input.stick_y    = GC_AXIS_CENTER;
        gc_input.cstick_x   = GC_AXIS_CENTER;
        gc_input.cstick_y   = GC_AXIS_CENTER;
        gc_input.trigger_l = 0;
        gc_input.trigger_r = 0;
    }
    else
    {
        // Generate the USB Data for GameCube native mode
        gc_input.button_a   = poll_response->button_a;
        gc_input.button_b   = poll_response->button_b;

        gc_input.dpad_down  = poll_response->dpad_down;
        gc_input.dpad_up    = poll_response->dpad_up;
        gc_input.dpad_left  = poll_response->dpad_left;
        gc_input.dpad_right = poll_response->dpad_right;

        gc_input.button_start   = poll_response->button_start;

        // Defaults
        gc_input.button_y       = poll_response->button_y;
        gc_input.button_x       = poll_response->button_x;
        gc_input.button_z       = poll_response->button_z;

        if (adapter_settings.zjump == 1)
        {
            gc_input.button_x       = poll_response->button_z;
            gc_input.button_z       = poll_response->button_x;
        }
        else if (adapter_settings.zjump == 2)
        {
            gc_input.button_y       = poll_response->button_z;
            gc_input.button_z       = poll_response->button_y;
        }

        int adj_tl  = gc_origin_adjust(poll_response->trigger_l, origin_data->trigger_l,  false);
        int adj_tr  = gc_origin_adjust(poll_response->trigger_r, origin_data->trigger_r,  false);

        switch( adapter_settings.trigger_mode_l )
        {
            default:
            case TRIG_MODE_OFF:
                gc_input.trigger_l          = scale_trigger(adj_tl);
                gc_input.button_l           = poll_response->button_l;
                break;

            case TRIG_MODE_A2D:
                gc_input.trigger_l          = (adj_tl >= adapter_settings.trigger_threshold_l) ? 255 : scale_trigger(adj_tl);
                gc_input.button_l           = (adj_tl >= adapter_settings.trigger_threshold_l) ? 1 : 0;
                break;

            case TRIG_MODE_D2ALITE:
                gc_input.trigger_l          = (poll_response->button_l) ? 85 : 0;
                gc_input.button_l           = 0;
                break;

            case TRIG_MODE_D2AFULL:
                gc_input.trigger_l          = (poll_response->button_l) ? 255 : 0;
                gc_input.button_l           = poll_response->button_l;
                break;
        }

        switch( adapter_settings.trigger_mode_r )
        {
            default:
            case TRIG_MODE_OFF:
                gc_input.trigger_r      = scale_trigger(adj_tr);
                gc_input.button_r       = poll_response->button_r;
                break;

            case TRIG_MODE_A2D:
                gc_input.trigger_r      = (adj_tr >= adapter_settings.trigger_threshold_r) ? 255 : scale_trigger(adj_tr);
                gc_input.button_r       = (adj_tr >= adapter_settings.trigger_threshold_r) ? 1 : 0;
                break;

            case TRIG_MODE_D2ALITE:
                gc_input.trigger_r          = (poll_response->button_r) ? 85 : 0;
                gc_input.button_r           = 0;
                break;

            case TRIG_MODE_D2AFULL:
                gc_input.trigger_r          = (poll_response->button_r) ? 255 : 0;
                gc_input.button_r           = poll_response->button_r;
                break;
        }

        int adj_x   = gc_origin_adjust(poll_response->stick_x,  origin_data->stick_x,     false);
        int adj_y   = gc_origin_adjust(poll_response->stick_y,  origin_data->stick_y,     false);
        int adj_cx  = gc_origin_adjust(poll_response->cstick_x, origin_data->cstick_x,    false);
        int adj_cy  = gc_origin_adjust(poll_response->cstick_y, origin_data->cstick_y,    false);

        gc_input.stick_x        = adj_x;
        gc_input.stick_y        = adj_y;
        gc_input.cstick_x       = adj_cx;
        gc_input.cstick_y       = adj_cy;
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
