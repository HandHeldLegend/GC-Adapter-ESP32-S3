#include "rgb_animator.h"

TaskHandle_t _rgb_task_handle = NULL;
#define REFRESH_RATE_ANIM 16 //16 hz = ~60fps

rgb_s start_color    = {0};
rgb_s current_color  = {0};
rgb_s target_color   = {0};

int frames_left = 0;
uint16_t total_frames = 0;
bool rgb_lock = false;

rgb_s colors[CONFIG_NP_RGB_COUNT];

// This will override any current animation!
void rgb_animate_to(rgb_s target_color, uint16_t frames)
{
    rgb_lock = true;
    target_color.rgb = target_color.rgb;
    total_frames = frames;
    frames_left = (int) frames;
    rgb_lock = false;
}

void rgb_animator_task(void * params)
{
    for(;;)
    {
        if (!rgb_lock)
        {
            vTaskDelay(REFRESH_RATE_ANIM/portTICK_PERIOD_MS);
            continue;
        }
        if (frames_left > 0)
        {
            float ratio = (float) frames_left / (float) total_frames;
            float fader = 255 * ratio;
            current_color = rgb_blend(start_color, target_color, 255 - (uint8_t) fader);
            rgb_setall(COLOR_RED);
            rgb_show();
            frames_left -= 1;
        }
        else if (start_color.rgb != target_color.rgb)
        {
            start_color.rgb = target_color.rgb;
            current_color.rgb = target_color.rgb;
            rgb_setall(current_color);
            rgb_show(); 
        }
        vTaskDelay(REFRESH_RATE_ANIM/portTICK_PERIOD_MS);
    }

}

void rgb_animator_init()
{
    const char* TAG = "rgb_animator_init";

    neopixel_init(colors, SPI3_HOST);
    rgb_setbrightness(adapter_settings.led_brightness);

    if (_rgb_task_handle != NULL)
    {
        vTaskDelete(_rgb_task_handle);
        ESP_LOGI(TAG, "Animator already running... restarting.");
        _rgb_task_handle = NULL;
    }

    xTaskCreatePinnedToCore(rgb_animator_task, "anim task", 6048, NULL, 2, &_rgb_task_handle, 0);
}