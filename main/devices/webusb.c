#include "webusb.h"

uint8_t _webusb_out_buffer[64] = {0x00};

void webusb_save_confirm()
{
    printf("Sending Save receipt...\n");
    memset(_webusb_out_buffer, 0, 64);
    _webusb_out_buffer[0] = 0xF1;
    tud_vendor_n_write(0, _webusb_out_buffer, 64);
    tud_vendor_n_flush(0);
}

void webusb_command_processor(uint8_t *data)
{
    switch(data[0])
    {
        default:

            break;

        case WEBUSB_CMD_FW_GET:
            {
                _webusb_out_buffer[0] = WEBUSB_CMD_FW_GET;
                _webusb_out_buffer[1] = (FIRMWARE_VERSION & 0xFF00)>>8;
                _webusb_out_buffer[2] = FIRMWARE_VERSION & 0xFF;
                tud_vendor_n_write(0, _webusb_out_buffer, 64);
                tud_vendor_n_flush(0);
            }
            break;

        // Set RGB Group
        case WEBUSB_CMD_RGB_SET:
            {
                // TODO
            }
            break;

        case WEBUSB_CMD_RGB_GET:
            {
                printf("WebUSB: Got RGB GET command.\n");
                memset(_webusb_out_buffer, 0, 64);
                _webusb_out_buffer[0] = WEBUSB_CMD_RGB_GET;

                // TODO

                tud_vendor_n_write(0, _webusb_out_buffer, 64);
                tud_vendor_n_flush(0);
            }
            break;

        case WEBUSB_CMD_SAVEALL:
            {
                save_adapter_settings();
                rgb_animate_blink(COLOR_GREEN);
                vTaskDelay(250/portTICK_PERIOD_MS);
                //settings_save_webindicate();
                gamecube_rmt_init();
            }
            break;
    }
}
