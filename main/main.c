/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "adapter_includes.h"
#include "descriptors.h"
static const char *TAG = "Mitch GC Pro Adapter";

rgb_s colors[CONFIG_NP_RGB_COUNT];

TaskHandle_t mode_task_handle = NULL;
bool mode_change_toggle = false;
bool mode_button_pressed = false;

uint8_t hue = 0;
uint8_t sat = 255;
uint8_t val = 255;


void app_main(void)
{
    // Initialize button that will trigger HID reports
    const gpio_config_t boot_button_config = {
        .pin_bit_mask = PIN_MASK_GCP,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = true,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&boot_button_config));

    const gpio_config_t led_latency_pin_config = {
        .pin_bit_mask = LED_PIN_MASK,
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = false,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&led_latency_pin_config));

    vTaskDelay(500/portTICK_PERIOD_MS);

    load_adapter_settings();

    uint32_t regread = REG_READ(GPIO_IN_REG) & PIN_MASK_GCP;
    if (!util_getbit(regread, PREV_BUTTON))
    {
        active_usb_mode = USB_MODE_GENERIC;
    }
    else
    {
        active_usb_mode = adapter_settings.adapter_mode;
        // Start task which will help with mode switching
        xTaskCreatePinnedToCore(adapter_mode_task, "mode_task", 4000, NULL, 4, &mode_task_handle, 1);
    }

    neopixel_init(colors, SPI3_HOST);
    rgb_setbrightness(adapter_settings.led_brightness);
    rgb_animator_init();

    cmd_phase = CMD_PHASE_PROBE;

    switch(active_usb_mode)
    {
        default:
        case USB_MODE_NS:
            mode_color.rgb = COLOR_YELLOW.rgb;
        break;
        case USB_MODE_GC:
            mode_color.rgb = COLOR_PURPLE.rgb;
        break;
        case USB_MODE_GENERIC:
            mode_color.rgb = COLOR_BLUE.rgb;
        break;
        case USB_MODE_XINPUT:
            mode_color.rgb = 0x107C10;
            break;
    }
    rgb_animate_to(mode_color);

    vTaskDelay(250/portTICK_PERIOD_MS);

    gc_timer_init();
    gamecube_rmt_init();
    
    gcusb_start(active_usb_mode);
}
