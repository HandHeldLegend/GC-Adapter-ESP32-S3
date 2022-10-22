/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "gc.h"
#include "rmt_gc.h"

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
static const char *TAG = "Mitch GC Pro Adapter";

TaskHandle_t usb_task_handle = NULL;
QueueHandle_t usb_queue = NULL;

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

/**
 * @brief HID report descriptor
 *
 * In this example we implement Keyboard + Mouse HID device,
 * so we must define both report descriptors
 */
const uint8_t hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x25, 0x07,        //   Logical Maximum (7)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x09, 0x39,        //   Usage (Hat switch)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x46, 0xFF, 0x00,  //   Physical Maximum (255)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x0C,        //   Usage Page (Consumer)
    0x09, 0x00,        //   Usage (Unassigned)
    0x15, 0x80,        //   Logical Minimum (-128)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection

    // 94 bytes
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and 1 HID interface
 */
static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(hid_report_descriptor), 0x81, 8, 1),
};

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

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

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

/********* Application ***************/
void usb_send_task(void *parameters) 
{

    while(1)
    {
        vTaskDelay(0.15/portTICK_PERIOD_MS);
        if (rx_recieved)
        {
            rx_recieved = false;
            switch(cmd_phase)
            {
                default:
                case CMD_PHASE_PROBE:
                    
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

                    ESP_LOGI("CMD", "PROBE GOT: %X", (unsigned int) gc_probe_response.id_upper);
                    if (gc_probe_response.id_upper == 0x9)
                    {
                        cmd_phase = CMD_PHASE_ORIGIN;
                        memcpy(JB_TX_MEM, gcmd_origin_rmt, sizeof(rmt_item32_t) * GCMD_ORIGIN_LEN);
                    }

                    // Set the memory owner back appropriately.
                    JB_RX_MEMOWNER  = 1;

                    // Reset RX memory pointer
                    JB_RX_RDRST     = 1;
                    JB_RX_RDRST     = 0;

                    // Set RX to begin so it starts when sync bit is set.
                    JB_RX_BEGIN     = 1;
                    // Start next transaction.
                    JB_TX_BEGIN     = 1;
                    break;

                case CMD_PHASE_ORIGIN:
                    
                case CMD_PHASE_POLL:
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

                    if (cmd_phase == CMD_PHASE_ORIGIN)
                    {
                            //ESP_LOGI("CMD", "ORIGIN GOT");
                            gc_origin_data.data_set = true;

                            gc_origin_data.stick_x  = 128 - (int) gc_poll_response.stick_x;
                            gc_origin_data.stick_y  = 128 - (int) gc_poll_response.stick_y;
                            gc_origin_data.cstick_x = 128 - (int) gc_poll_response.cstick_x;
                            gc_origin_data.cstick_y = 128 - (int) gc_poll_response.cstick_y;

                            memcpy(JB_TX_MEM, gcmd_poll_rmt, sizeof(rmt_item32_t) * GCMD_POLL_LEN);

                            cmd_phase = CMD_PHASE_POLL;
                            JB_RX_MEMOWNER  = 1;
                            JB_RX_BEGIN     = 1;
                            JB_TX_BEGIN     = 1;
                    }
                    else
                    {
                        //ESP_LOGI("CMD", "POLL GOT");
                        // Handle USB update
                        gamecube_send_usb();
                        tud_hid_report(0, &usb_buffer, 8);
                        vTaskDelay(0.5/portTICK_PERIOD_MS);
                        JB_RX_MEMOWNER  = 1;
                        JB_RX_BEGIN     = 1;
                        JB_TX_BEGIN     = 1;
                    }

                    break;
            }
            
        }
        else
        {
            rx_timeout+=1;
            if (rx_timeout > 1000)
            {
                ESP_LOGI("TIMEOUT", "RX TIMEOUT");

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
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // Initialize button that will trigger HID reports
    const gpio_config_t boot_button_config = {
        .pin_bit_mask = BIT64(APP_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = true,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&boot_button_config));

    gamecube_reader_start();

    xTaskCreatePinnedToCore(usb_send_task, "usb_task", 4048, NULL, 0, &usb_task_handle, 1);
}
