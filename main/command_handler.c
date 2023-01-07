#include "command_handler.h"

void command_handler(const uint8_t *data, uint16_t bufsize)
{
    switch (data[1])
    {
        // Reset settings to default
        case CMD_SETTINGS_DEFAULT:
            load_adapter_defaults();
            break;

        // Save all settings
        case CMD_SETTINGS_SAVEALL:
            save_adapter_mode();
            break;

        // Load all settings to web interface
        case CMD_SETTINGS_GETALL:
            uint8_t buffer[CMD_USB_REPORTLEN] = {0};
            buffer[0] = CMD_USB_REPORTID;
            buffer[1] = CMD_SETTINGS_GETALL;
            adapter_settings.magic_num = MAGIC_NUM;

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
            switch (data[2])
            {
                default:
                case USB_MODE_GENERIC:
                    if (!data[3])
                    {
                        adapter_settings.di_trigger_l = data[4];
                    }
                    else
                    {
                        adapter_settings.di_trigger_r = data[4];
                    }
                break;

                case USB_MODE_NS:
                    if (!data[3])
                    {
                        adapter_settings.ns_trigger_l = data[4];
                    }
                    else
                    {
                        adapter_settings.ns_trigger_r = data[4];
                    }
                break;

                case USB_MODE_GC:
                    if (!data[3])
                    {
                        adapter_settings.gc_trigger_l = data[4];
                    }
                    else
                    {
                        adapter_settings.gc_trigger_r = data[4];
                    }
                break;

                case USB_MODE_XINPUT:
                    if (!data[3])
                    {
                        adapter_settings.xi_trigger_l = data[4];
                    }
                    else
                    {
                        adapter_settings.xi_trigger_r = data[4];
                    }
                break;
            }
            break;

        // Set trigger sensitivity
        case CMD_SETTINGS_TRIGGERSENSITIVITY:
            if (!data[2])
            {
                adapter_settings.trigger_threshold_l = data[3];
            }
            else
            {
                adapter_settings.trigger_threshold_r = data[3];
            }
            break;
    }
}