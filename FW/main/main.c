#include "main.h"

bool cb_adapter_hardware_test()
{
    adapter_ll_hardware_setup();
    rgb_init();

    // Flash all three rgb colors then we can return our test value
    rgb_set_instant(COLOR_RED.color);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    rgb_set_instant(COLOR_GREEN.color);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    rgb_set_instant(COLOR_BLUE.color);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    rgb_set_instant(COLOR_WHITE.color);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    rgb_set_instant(0x00);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    return true;
}

void app_main(void)
{

    /*const gpio_config_t led_latency_pin_config = {
        .pin_bit_mask = LED_PIN_MASK,
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = false,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&led_latency_pin_config));*/

    
    vTaskDelay(100/portTICK_PERIOD_MS);


    adapter_main_init();

    adapter_main_loop();

}
