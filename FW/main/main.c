#include "main.h"

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
