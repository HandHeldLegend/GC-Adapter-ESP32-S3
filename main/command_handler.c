#include "command_handler.h"

bool cmd_flagged = false;
uint8_t cmd_queue[8] = {0};
uint8_t cmd_queue_idx = 0;
uint8_t cmd_buffer[CMD_USB_REPORTLEN] = {0};

void command_queue_process()
{
    uint8_t command = cmd_queue[cmd_queue_idx];

    memset(cmd_buffer, 0, CMD_USB_REPORTLEN);
    cmd_buffer[0] = command;

    switch(command)
    {
        case CMD_SETTINGS_LEDBRIGHTNESS:
            cmd_buffer[1] = adapter_settings.led_brightness;
            break;

        case CMD_SETTINGS_TRIGGERMODE:
            memcpy(&cmd_buffer[1], &adapter_settings.trigger_mode, 2);
            break;

        case CMD_SETTINGS_TRIGGERSENSITIVITY:
            cmd_buffer[1] = adapter_settings.trigger_threshold_l;
            cmd_buffer[2] = adapter_settings.trigger_threshold_r;
            break;

        case CMD_SETTINGS_ZJUMP:
            cmd_buffer[1] = adapter_settings.zjump;
            break;

        case CMD_SETTINGS_SETTINGVERSION:
            memcpy(&cmd_buffer[1], &adapter_settings.settings_version, 2);
            break;

        case CMD_SETTINGS_FWVERSION:
            uint16_t tmp = FIRMWARE_VERSION;
            memcpy(&cmd_buffer[1], &tmp, 2);
            break;
    }

    if (!cmd_queue_idx)
    {
        cmd_flagged = false;
        
    }
    else
    {
        cmd_queue_idx -= 1;
    }

    tud_hid_report(CMD_USB_REPORTID, cmd_buffer, CMD_USB_REPORTLEN);
}

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
            save_adapter_settings();
            rgb_animate_blink(COLOR_GREEN);
            vTaskDelay(250/portTICK_PERIOD_MS);
            gamecube_rmt_init();
            usb_send_data();
            break;

        // Load all settings to web interface
        case CMD_SETTINGS_GETALL:
            
            // Set up our command queue to
            // send out the appropriate commands.
            cmd_queue[0] = CMD_SETTINGS_LEDBRIGHTNESS;
            cmd_queue[1] = CMD_SETTINGS_TRIGGERMODE;
            cmd_queue[2] = CMD_SETTINGS_TRIGGERSENSITIVITY;
            cmd_queue[3] = CMD_SETTINGS_ZJUMP;
            cmd_queue[4] = CMD_SETTINGS_SETTINGVERSION;
            cmd_queue[5] = CMD_SETTINGS_FWVERSION;
            cmd_queue_idx = 5;
            cmd_flagged = true;
            break;

        // Set LED brightness
        case CMD_SETTINGS_LEDBRIGHTNESS:
            uint8_t new = data[2];
            if (!new)
            {
                new = 1;
            }
            rgb_setbrightness(new);
            adapter_settings.led_brightness = new;
            rgb_show();
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

        // Set ZJump setting
        case CMD_SETTINGS_ZJUMP:
            switch(data[2])
            {
                default:
                case USB_MODE_GENERIC:
                    adapter_settings.di_zjump = data[3];
                    break;

                case USB_MODE_GC:
                    adapter_settings.gc_zjump = data[3];
                    break;

                case USB_MODE_NS:
                    adapter_settings.ns_zjump = data[3];
                    break;

                case USB_MODE_XINPUT:
                    adapter_settings.xi_zjump = data[3];
                    break;
            }
            break;
    }
}