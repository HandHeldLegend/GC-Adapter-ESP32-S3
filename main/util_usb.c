#include "util_usb.h"
#include "descriptors.h"

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{

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
            return generic_hid_report_descriptor;
            break;
    }
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
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

esp_err_t gcusb_start(usb_mode_t mode)
{
    const char* TAG = "gcusb_start";

    if (adapter_status != GCSTATUS_IDLE)
    {
        gcusb_stop();
    }

    ESP_LOGI(TAG, "USB initialization");
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    switch (adapter_mode)
    {
        default:
        case USB_MODE_NS:
            tusb_cfg.configuration_descriptor = ns_hid_configuration_descriptor;
            break;
        case USB_MODE_GC:
            //tusb_cfg.configuration_descriptor = gc_hid_configuration_descriptor;
            break;
        case USB_MODE_GENERIC:
            //tusb_cfg.configuration_descriptor = generic_hid_configuration_descriptor;
            break;
    }

    return ESP_OK;
}

void gcusb_stop()
{
    adapter_status = GCSTATUS_IDLE;
}

void gcusb_send_data()
{
    if (adapter_status == GCSTATUS_IDLE)
    {
        return;
    }

    switch (adapter_mode)
    {
        default:
        case USB_MODE_NS:
            // Generate the USB Data for NS mode
            ns_input.button_a = gc_poll_response.button_a;
            ns_input.button_b = gc_poll_response.button_b;
            ns_input.button_x = gc_poll_response.button_x;
            ns_input.button_y = gc_poll_response.button_y;

            uint8_t lr = 1 - gc_poll_response.button_dl + gc_poll_response.button_dr;
            uint8_t ud = 1 - gc_poll_response.button_dd + gc_poll_response.button_du;

            ns_input.dpad_hat = dir_to_hat(lr, ud);

            ns_input.button_plus = gc_poll_response.button_start;

            ns_input.trigger_r = gc_poll_response.button_z;

            ns_input.trigger_zl = gc_poll_response.button_l;
            ns_input.trigger_zr = gc_poll_response.button_r;
            
            int adj_x   = (int) gc_poll_response.stick_x + gc_origin_data.stick_x;
            int adj_y   = 256 - (int) gc_poll_response.stick_y + gc_origin_data.stick_y;
            int adj_cx  = (int) gc_poll_response.cstick_x + gc_origin_data.cstick_x;
            int adj_cy  = (int) gc_poll_response.cstick_y + gc_origin_data.cstick_y;

            ns_input.stick_left_x   = (uint8_t) adj_x;
            ns_input.stick_left_y   = (uint8_t) adj_y;
            ns_input.stick_right_x  = (uint8_t) adj_cx;
            ns_input.stick_right_y  = (uint8_t) adj_cy;

            memcpy(&ns_buffer, &ns_input, NS_HID_LEN);

            // Send USB report
            tud_hid_report(0, &ns_buffer, 8);
            break;
        case USB_MODE_GC:

            break;
        case USB_MODE_GENERIC:

            break;
    }
}