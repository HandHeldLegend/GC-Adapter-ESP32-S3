/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "adapter_includes.h"
#include "descriptors.h"

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
static const char *TAG = "Mitch GC Pro Adapter";

rgb_s led_colors[GC_LED_COUNT] = {COLOR_RED};
TaskHandle_t usb_task_handle = NULL;
bool mode_change_toggle = false;
bool mode_button_pressed = false;
#if ADAPTER_DEBUG_ENABLE
TaskHandle_t debug_task_handle = NULL;
#endif

uint8_t hue = 0;
uint8_t sat = 255;
uint8_t val = 255;

void debug_task(void *parameters)
{
    ESP_LOGI("DEBUG:", "Starting Debug Task Loop");
    cmd_phase = CMD_PHASE_DEBUG;
    while(1)
    {
        // Simulate controller connected every 1 ms
        rx_recieved = 1;
        vTaskDelay(8/portTICK_PERIOD_MS);
    }
}
bool dtoggle = false;
/********* Application ***************/
void main_gamecube_task(void *parameters) 
{
    // Loop until task gets killed.
    while(1)
    {
        // We delay to give task time for other things :)
        vTaskDelay(0.05/portTICK_PERIOD_MS);

        // If we got a reply from a GameCube Controller, process it.
        if (rx_recieved)
        {
            // Clear the rx_received flag.
            rx_recieved = false;

            // Check which part of the cmd_phase we are in.
            switch(cmd_phase)
            {
                case CMD_PHASE_DEBUG:
                    uint8_t b = !gpio_get_level(APP_BUTTON);
                    gc_poll_response.button_z = b;
                    gc_poll_response.button_a = b;
                    if (adapter_status == GCSTATUS_WORKING)
                    {
                        gcusb_send_data(false);
                    }
                    break;

                default:
                case CMD_PHASE_PROBE:
                    if (rx_offset != GC_PROBE_RESPONSE_LEN)
                    {
                        ESP_LOGI("CMDPHASE", "Command Phase Reponse Length Error.");
                    }
                    else
                    {
                        ESP_LOGI("CMDPHASE", "Command Phase got OK.");
                        // Clear our probe response
                        memset(&gc_probe_response, 0, sizeof(gc_probe_response_s));

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_probe_response.id_upper |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_probe_response.id_lower |= ((JB_RX_MEM[i+8].duration0 < JB_RX_MEM[i+8].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_probe_response.junk |= ((JB_RX_MEM[i+16].duration0 < JB_RX_MEM[i+16].duration1) ? 1 : 0) << (7-i);
                        }

                        if (gc_probe_response.id_upper == 0x9)
                        {
                            cmd_phase = CMD_PHASE_ORIGIN;
                            memcpy(JB_TX_MEM, gcmd_origin_rmt, sizeof(rmt_item32_t) * GCMD_ORIGIN_LEN);
                        }
                    }
                    // Set the memory owner back appropriately.
                    JB_RX_MEMOWNER  = 1;
                    // Set RX to begin so it starts when sync bit is set.
                    JB_RX_BEGIN     = 1;
                    // Start next transaction.
                    JB_TX_BEGIN     = 1;
                    break;

                case CMD_PHASE_ORIGIN:
                    if (rx_offset != GC_ORIGIN_RESPONSE_LEN)
                    {
                        ESP_LOGI("ORIGINPHASE", "Command Phase Reponse Length Error.");
                    }
                    else
                    {

                        memset(&gc_poll_response, 0, 3);

                        for (uint8_t i = 0; i < 3; i++)
                        {
                            gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                        }

                        // Toss out junk data
                        if (gc_poll_response.b1blank != 0x00)
                        {
                            break;
                        }

                        memset(&gc_poll_response, 0, sizeof(gc_poll_response));

                        ESP_LOGI("ORIGINPHASE", "Got Origin Response OK.");

                        for (uint8_t i = 3; i < 8; i++)
                        {
                            gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.buttons_2 |= ((JB_RX_MEM[i+8].duration0 < JB_RX_MEM[i+8].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.stick_x |= ((JB_RX_MEM[i+16].duration0 < JB_RX_MEM[i+16].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.stick_y |= ((JB_RX_MEM[i+24].duration0 < JB_RX_MEM[i+24].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.cstick_x |= ((JB_RX_MEM[i+32].duration0 < JB_RX_MEM[i+32].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.cstick_y |= ((JB_RX_MEM[i+40].duration0 < JB_RX_MEM[i+40].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.trigger_l |= ((JB_RX_MEM2[i].duration0 < JB_RX_MEM2[i].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.trigger_r |= ((JB_RX_MEM2[i+8].duration0 < JB_RX_MEM2[i+8].duration1) ? 1 : 0) << (7-i);
                        }

                        gc_origin_data.data_set = true;

                        gc_origin_data.stick_x  = 128 - (int) gc_poll_response.stick_x;
                        gc_origin_data.stick_y  = 128 - (int) gc_poll_response.stick_y;
                        gc_origin_data.cstick_x = 128 - (int) gc_poll_response.cstick_x;
                        gc_origin_data.cstick_y = 128 - (int) gc_poll_response.cstick_y;

                        memcpy(JB_TX_MEM, gcmd_poll_rmt, sizeof(rmt_item32_t) * GCMD_POLL_LEN);

                        rgb_setall(COLOR_GREEN, led_colors);
                        rgb_show();

                        cmd_phase = CMD_PHASE_POLL;
                    }
                    JB_RX_MEMOWNER  = 1;
                    JB_RX_BEGIN     = 1;
                    JB_TX_BEGIN     = 1;
                    
                case CMD_PHASE_POLL:
                    
                    memset(&gc_poll_response, 0, 3);

                    for (uint8_t i = 0; i < 3; i++)
                    {
                        gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                    }

                    // Toss out junk data
                    if (!gc_poll_response.b1blank && rx_offset == GC_POLL_RESPONSE_LEN)
                    {
                        rx_offset = 0;
                        memset(&gc_poll_response, 0, sizeof(gc_poll_response));

                        for (uint8_t i = 3; i < 8; i++)
                        {
                            gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.buttons_2 |= ((JB_RX_MEM[i+8].duration0 < JB_RX_MEM[i+8].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.stick_x |= ((JB_RX_MEM[i+16].duration0 < JB_RX_MEM[i+16].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.stick_y |= ((JB_RX_MEM[i+24].duration0 < JB_RX_MEM[i+24].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.cstick_x |= ((JB_RX_MEM[i+32].duration0 < JB_RX_MEM[i+32].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.cstick_y |= ((JB_RX_MEM[i+40].duration0 < JB_RX_MEM[i+40].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.trigger_l |= ((JB_RX_MEM2[i].duration0 < JB_RX_MEM2[i].duration1) ? 1 : 0) << (7-i);
                        }

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            gc_poll_response.trigger_r |= ((JB_RX_MEM2[i+8].duration0 < JB_RX_MEM2[i+8].duration1) ? 1 : 0) << (7-i);
                        }

                        if (adapter_status == GCSTATUS_WORKING)
                        {
                            gcusb_send_data(false);
                        }

                    }
                    else
                    {
                        if (adapter_status == GCSTATUS_WORKING)
                        {
                            gcusb_send_data(true);
                        }
                    }

                    JB_RX_MEMOWNER  = 1;
                    JB_RX_BEGIN     = 1;
                    JB_TX_BEGIN     = 1;

                    break;
            } 
        }
        else
        {
            rx_timeout+=1;
            if (rx_timeout > 1000)
            {
                rx_timeout = 0;
                rgb_setall(COLOR_RED, led_colors);
                rgb_show();
                memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GCMD_PROBE_LEN);
                #if ADAPTER_DEBUG_ENABLE
                cmd_phase = CMD_PHASE_DEBUG;
                #else
                cmd_phase = CMD_PHASE_PROBE;
                #endif

                JB_RX_MEMOWNER  = 1;
                JB_RX_RDRST     = 1;
                JB_RX_RDRST     = 0;
                JB_RX_BEGIN     = 0;
                JB_RX_SYNC      = 1;
                JB_RX_CLEARISR  = 1;
                JB_RX_BEGIN     = 1;

                JB_TX_RDRST     = 1;
                JB_TX_WRRST     = 1;
                JB_TX_CLEARISR  = 1;
                JB_TX_BEGIN     = 1;
            }

            if (!gpio_get_level(APP_BUTTON) && !mode_button_pressed)
            {
                mode_button_pressed = true;
            }
            else if (gpio_get_level(APP_BUTTON) && mode_button_pressed)
            {
                mode_change_toggle = true;
            }

            if (mode_change_toggle)
            {
                mode_change_toggle = false;
                adapter_mode += 1;
                if (adapter_mode == USB_MODE_MAX)
                {
                    adapter_mode = USB_MODE_NS;
                }
                save_adapter_mode();
                esp_restart();
            }
            
        }
    }
}

void app_main(void)
{
    // Initialize button that will trigger HID reports
    const gpio_config_t boot_button_config = {
        .pin_bit_mask = BIT64(APP_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = true,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&boot_button_config));
    vTaskDelay(500/portTICK_PERIOD_MS);

    load_adapter_mode();

    util_rgb_init(led_colors, RGB_MODE_GRB);
    rgb_setbrightness(255);

    cmd_phase = CMD_PHASE_PROBE;

    switch(adapter_mode)
    {
        default:
        case USB_MODE_NS:
            rgb_setall(COLOR_YELLOW, led_colors);
        break;
        case USB_MODE_GC:
            rgb_setall(COLOR_PURPLE, led_colors);
        break;
        case USB_MODE_GENERIC:
            rgb_setall(COLOR_BLUE, led_colors);
        break;
    }
    
    rgb_show();

    gcusb_start(adapter_mode);
    gamecube_reader_start();
    xTaskCreatePinnedToCore(main_gamecube_task, "gc_task", 6000, NULL, 0, &usb_task_handle, 1);

    #if ADAPTER_DEBUG_ENABLE
    vTaskDelay(1000/portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(debug_task, "debug_task", 2048, NULL, 0, &debug_task_handle, 0);
    #endif
}
