/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "adapter_includes.h"

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
static const char *TAG = "Mitch GC Pro Adapter";

TaskHandle_t usb_task_handle = NULL;
rgb_s colors[GC_LED_COUNT] = {COLOR_RED};

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

uint8_t hue = 0;
uint8_t sat = 255;
uint8_t val = 255;

/********* Application ***************/
void main_gamecube_task(void *parameters) 
{
    // Loop until task gets killed.
    while(1)
    {
        // We delay to give task time for other things :)
        vTaskDelay(0.15/portTICK_PERIOD_MS);
        //rgb_setall(rgb_from_hsv(hue, sat, val), colors);
        //rgb_show();
        //hue+=1;

        // If we got a reply from a GameCube Controller, process it.
        if (rx_recieved)
        {
            // Clear the rx_received flag.
            rx_recieved = false;

            // Check which part of the cmd_phase we are in.
            switch(cmd_phase)
            {
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
                        ESP_LOGI("ORIGINPHASE", "Got Origin Response OK.");
                        memset(&gc_poll_response, 0, sizeof(gc_poll_response_s));

                        for (uint8_t i = 0; i < 8; i++)
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

                        cmd_phase = CMD_PHASE_POLL;
                    }
                    JB_RX_MEMOWNER  = 1;
                    JB_RX_BEGIN     = 1;
                    JB_TX_BEGIN     = 1;
                    
                case CMD_PHASE_POLL:
                    if (rx_offset != GC_POLL_RESPONSE_LEN)
                    {
                        ESP_LOGI("POLLPHASE", "Poll Phase Reponse Length Error.");
                    }
                    else
                    {
                        memset(&gc_poll_response, 0, sizeof(gc_poll_response_s));

                        for (uint8_t i = 0; i < 8; i++)
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
                        //gcusb_send_data()
                        
                        //vTaskDelay(0.1/portTICK_PERIOD_MS);
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
                memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GCMD_PROBE_LEN);
                cmd_phase = CMD_PHASE_PROBE;

                JB_RX_MEMOWNER  = 1;
                JB_RX_BEGIN     = 0;
                JB_RX_SYNC      = 1;
                
                JB_RX_BEGIN     = 1;
                JB_TX_BEGIN     = 1;
            }
            
        }
    
    }
}

void app_main(void)
{

    //ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // Initialize button that will trigger HID reports
    const gpio_config_t boot_button_config = {
        .pin_bit_mask = BIT64(APP_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = true,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&boot_button_config));
    vTaskDelay(250/portTICK_PERIOD_MS);

    util_rgb_init(colors, RGB_MODE_GRB);
    rgb_setbrightness(255);

    rgb_setall(COLOR_RED, colors);
    rgb_show();
    vTaskDelay(800/portTICK_PERIOD_MS);
    rgb_setall(COLOR_ORANGE, colors);
    rgb_show();
    vTaskDelay(800/portTICK_PERIOD_MS);
    rgb_setall(COLOR_YELLOW, colors);
    rgb_show();
    vTaskDelay(800/portTICK_PERIOD_MS);
    rgb_setall(COLOR_GREEN, colors);
    rgb_show();
    vTaskDelay(800/portTICK_PERIOD_MS);
    rgb_setall(COLOR_BLUE, colors);
    rgb_show();
    vTaskDelay(800/portTICK_PERIOD_MS);
    rgb_setall(COLOR_PURPLE, colors);
    rgb_show();
    vTaskDelay(800/portTICK_PERIOD_MS);
    

    gamecube_reader_start();

    xTaskCreatePinnedToCore(main_gamecube_task, "gc_task", 4048, NULL, 0, &usb_task_handle, 1);
}
