#include "util_usb.h"

TaskHandle_t usb_device_taskdef;

const char *global_string_descriptor[] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    ADAPTER_MANUFACTURER, // 1: Manufacturer
    ADATPER_PRODUCTNAME,  // 2: Product
    "000000",             // 3: Serials, should use chip ID
};

uint16_t usb_timeout_time = 0;

// Used to switch back and forth for 'performance mode'
// stops issues where 1ms polling is borked on Switch.
uint8_t usb_polling_rate = 8;

/********* TinyUSB HID callbacks ***************/
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)reqlen;

    return 0;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len)
{
    switch (active_usb_mode)
    {
    default:
    case USB_MODE_NS:
        if ((report[0] == 0x21) || (report[0] == 0x30) || (report[0] == 0x3F))
        {
            // usb_process_data();
        }
        break;
    case USB_MODE_XINPUT:
        if ((report[0] == 0x00) && (report[1] == XID_REPORT_LEN))
        {
            // usb_process_data();
        }

        break;

    case USB_MODE_GC:
        if (report[0] == 0x21)
        {
            // usb_process_data();
        }
        break;
    }
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    switch (active_usb_mode)
    {
    default:
    case USB_MODE_NS:
        if (!report_id && !report_type)
        {
            if (buffer[0] == SW_OUT_ID_RUMBLE)
            {
                rx_vibrate = true;
                rumble_translate(&buffer[2]);
            }
            else
            {
                switch_commands_future_handle(buffer[0], buffer, bufsize);
            }
        }
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
                    gamecube_rumble_en(true);
                }
                else
                {
                    gamecube_rumble_en(false);
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
    (void)instance;
    switch (active_usb_mode)
    {
    default:
    case USB_MODE_NS:
        return swpro_hid_report_descriptor;
        break;
    case USB_MODE_GC:
        return gc_hid_report_descriptor;
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

const tusb_desc_webusb_url_t desc_url =
    {
        .bLength = 3 + sizeof(ADAPTER_WEBUSB_URL) - 1,
        .bDescriptorType = 3, // WEBUSB URL type
        .bScheme = 1,         // 0: http, 1: https
        .url = ADAPTER_WEBUSB_URL};

//--------------------------------------------------------------------+
// WebUSB use vendor class
//--------------------------------------------------------------------+

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    // nothing to with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP)
        return true;

    switch (request->bmRequestType_bit.type)
    {
    case TUSB_REQ_TYPE_VENDOR:
        switch (request->bRequest)
        {
        case VENDOR_REQUEST_WEBUSB:
            // match vendor request in BOS descriptor
            // Get landing page url
            return tud_control_xfer(rhport, request, (void *)(uintptr_t)&desc_url, desc_url.bLength);

        case VENDOR_REQUEST_MICROSOFT:
            if (request->wIndex == 7)
            {
                // Get Microsoft OS 2.0 compatible descriptor
                uint16_t total_len;
                if (active_usb_mode == USB_MODE_NS)
                {
                    memcpy(&total_len, desc_ms_os_20 + 8, 2);
                    return tud_control_xfer(rhport, request, (void *)(uintptr_t)desc_ms_os_20, total_len);
                }
                else
                {
                    memcpy(&total_len, desc_ms_os_20_gamecube + 8, 2);
                    return tud_control_xfer(rhport, request, (void *)(uintptr_t)desc_ms_os_20_gamecube, total_len);
                }
            }
            else
            {
                return false;
            }

        default:
            break;
        }
        break;

    case TUSB_REQ_TYPE_CLASS:
        printf("Vendor Request: %x", request->bRequest);

        // response with status OK
        return tud_control_status(rhport, request);
        break;

    default:
        break;
    }

    // stall unknown request
    return false;
}

/**
 * @brief Takes in directions and outputs a byte output appropriate for
 * HID Hat usage.
 * @param hat_type hat_mode_t type - The type of controller you're converting for.
 * @param leftRight 0 through 2 (2 is right) to indicate the direction left/right
 * @param upDown 0 through 2 (2 is up) to indicate the direction up/down
 */
uint8_t dir_to_hat(uint8_t leftRight, uint8_t upDown)
{
    uint8_t ret = 0;
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

    if (input > 129)
    {
        float tmp = (float)input - SCALE_AXIS_CENTER;
        tmp = tmp * 1.28f;
        res = (int)tmp + SCALE_AXIS_CENTER;
    }
    else if (input < 127)
    {
        float tmp = SCALE_AXIS_CENTER - (float)input;
        tmp = tmp * 1.28f;
        res = SCALE_AXIS_CENTER - (int)tmp;
    }
    else
    {
        res = SCALE_AXIS_CENTER;
    }

    if (res > 255)
    {
        res = 255;
    }
    if (res < 0)
    {
        res = 0;
    }
    return (uint8_t)res;
}

short sign_axis(int input)
{
    uint8_t scaled = scale_axis(input);

    int start = (int)scaled - 127;
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
    return (short)start;
}

uint8_t scale_trigger(int input)
{
    if (input < 0)
    {
        return (uint8_t)0;
    }
    else if (input <= 255)
    {
        return (uint8_t)input;
    }
    else
    {
        return (uint8_t)255;
    }
}

uint8_t gc_origin_adjust(uint8_t value, int origin, bool invert)
{
    int out = 0;

    if (invert)
    {
        out = 255 - ((int)value - origin);
    }
    else
    {
        out = (int)value - origin;
    }

    if (out < 0)
    {
        out = 0;
    }
    else if (out > 255)
    {
        out = 255;
    }

    return (uint8_t)out;
}

void gcusb_start(usb_mode_t mode)
{
    const char *TAG = "gcusb_start";

    ESP_LOGI(TAG, "USB initialization");

    switch (mode)
    {
    default:
    case USB_MODE_NS:
        ESP_LOGI(TAG, "NS MODE");
        ESP_ERROR_CHECK(tinyusb_driver_install(&swpro_cfg));
        break;

    case USB_MODE_GC:
        ESP_LOGI(TAG, "GCC MODE");
        if (adapter_settings.performance_mode)
        {
            ESP_ERROR_CHECK(tinyusb_driver_install(&gc_cfg_perf));
        }
        else
        {
            ESP_ERROR_CHECK(tinyusb_driver_install(&gc_cfg));
        }
        break;

    case USB_MODE_XINPUT:
        ESP_LOGI(TAG, "LEGACY XINPUT MODE");
        ESP_ERROR_CHECK(xinput_driver_install());
        break;
    }

    while (!tud_mounted())
    {
        vTaskDelay(8 / portTICK_PERIOD_MS);
    }
    if (mode != USB_MODE_XINPUT)
    {
        while (!tud_hid_ready())
        {
            vTaskDelay(8 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        while (!tud_xinput_ready())
        {
            vTaskDelay(8 / portTICK_PERIOD_MS);
        }
    }
    vTaskDelay(250 / portTICK_PERIOD_MS);
    // usb_send_data();

    // DEBUG
    for(;;)
    {
        usb_process_task();
    }
}

void usb_send_data(void)
{
    if(!tud_hid_ready()) 
    {
        vTaskDelay(4/portTICK_PERIOD_MS);
        return;
    }
    // Send USB data according to the adapter mode
    switch (active_usb_mode)
    {
    default:
    case USB_MODE_NS:
        swpro_hid_report(&gc_poll_response, &gc_origin_data);
        break;
    case USB_MODE_GC:
        gamecube_hid_report(&gc_poll_response, &gc_origin_data);
        break;
    case USB_MODE_XINPUT:
        xinput_report(&gc_poll_response, &gc_origin_data);
        break;
    }
}

void rmt_reset()
{
    if (cmd_phase == CMD_PHASE_POLL)
    {
        
        if (active_gc_type == GC_TYPE_WAVEBIRD)
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = JB_LOW;
        }
        else
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = JB_HIGH;//(rx_vibrate == true) ? JB_HIGH : JB_LOW;
        }
    }

    JB_RX_MEMOWNER = 1;
    JB_RX_RDRST = 1;
    JB_RX_RDRST = 0;
    JB_RX_CLEARISR = 1;
    JB_RX_BEGIN = 0;
    JB_RX_SYNC = 1;
    JB_RX_SYNC = 0;
    JB_RX_BEGIN = 1;
    JB_TX_RDRST = 1;
    JB_TX_WRRST = 1;
    JB_TX_CLEARISR = 1;
}

void rmt_begin()
{
    // Start RMT transaction
    // Set mem owner
    JB_RX_MEMOWNER = 1;
    // Set RX to begin so it starts when sync bit is set.
    JB_RX_BEGIN = 1;
    // Start next transaction.
    JB_TX_BEGIN = 1;
}

// This is called after each successful USB report send.
void usb_process_task()
{
    usb_timeout_time = 0;

    if (switch_get_reporting_mode() == 0x30)
    {
        

        // Handle performance mode being disabled
        if (((active_usb_mode == USB_MODE_GC) && (!adapter_settings.performance_mode)) || (active_usb_mode == USB_MODE_NS))
        {
            // Poll 7 times every time
            for (uint8_t i = 0; i < 7; i++)
            {
                ets_delay_us(NON_P_MODE_DELAY);
                rmt_begin();
                ets_delay_us(TIMEOUT_GC_US);

                // If we timed out, just reset for next phase
                if (!rx_recieved)
                {

                    rmt_reset();

                    rx_timeout_counts += 1;
                    if (rx_timeout_counts >= TIMEOUT_COUNTS)
                    {
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
            }
        }
        // Default handling (Poll 1 time for 1ms interval)
        else
        {
            ets_delay_us(250);
            rmt_begin();
            ets_delay_us(TIMEOUT_GC_US);

            // If we timed out, just reset for next phase
            if (!rx_recieved)
            {

                rmt_reset();

                rx_timeout_counts += 1;
                if (rx_timeout_counts >= TIMEOUT_COUNTS)
                {
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
        }
    }

    // Send at the end
    usb_send_data();
}
