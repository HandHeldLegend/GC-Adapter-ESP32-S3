#include "command_handler.h"

void command_handler(const uint8_t *data, uint16_t bufsize)
{
    switch (data[1])
    {
        // Set all settings together
        case CMD_SETTINGS_BULKALL:
            
            break;
        // Reset settings to default
        case CMD_SETTINGS_DEFAULT:
            load_adapter_defaults();
            break;

        // Save all settings
        case CMD_SETTINGS_SAVEALL:
            memcpy(&adapter_settings, &data[2], sizeof(adapter_settings_s));
            save_adapter_mode();
            break;

        // Load all settings to web interface
        case CMD_SETTINGS_GETALL:
            uint8_t buffer[CMD_USB_REPORTLEN] = {0};
            buffer[0] = CMD_USB_REPORTID;
            buffer[1] = CMD_SETTINGS_GETALL;

            memcpy(&buffer[2], &adapter_settings, sizeof(adapter_settings_s));

            if (tud_hid_ready())
            {
                tud_hid_report(0, buffer, CMD_USB_REPORTLEN);
            }

            break;

        // Set LED brightness
        case CMD_SETTINGS_LEDBRIGHTNESS:
            rgb_setbrightness(data[2]);
            adapter_settings.led_brightness = data[2];
            rgb_show();
            break;

        // Set trigger mode
        case CMD_SETTINGS_TRIGGERMODE:
            memcpy(&adapter_settings.trigger_mode, &data[2], sizeof(uint16_t));
            break;

        // Set trigger sensitivity
        case CMD_SETTINGS_TRIGGERSENSITIVITY:
            adapter_settings.trigger_threshold_l = data[2];
            adapter_settings.trigger_threshold_r = data[3];
            break;
    }
}