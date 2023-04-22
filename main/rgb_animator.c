#include "rgb_animator.h"

rgb_s mode_color = {0};

rgb_s last_color = {0};
rgb_s current_color = {0};
rgb_s next_color = {0};

rgb_msg_s rgb_msg_fade = {0};
rgb_msg_s rgb_msg_blink = {0};

TaskHandle_t    rgb_animator_TaskHandle = NULL;
QueueHandle_t   rgb_animator_Queue;

bool rgb_blinking = false;

void rgb_animate_internal_fade(rgb_s color)
{
    uint8_t fader = 0;
    next_color.rgb = color.rgb;
    
    while(fader < 30)
    {
        uint8_t t = 0;
        if ((8 * fader) > 255)
        {
            t = 255;
        }
        else
        {
            t = (8 * fader);
        }
        rgb_blend(&current_color, last_color, next_color, t);
        rgb_setall(current_color);
        rgb_show();
        fader += 1;
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    last_color.rgb = next_color.rgb;
    rgb_setall(next_color);
    rgb_show();
}

void rgb_animate_internal_blink(rgb_s color)
{
    uint8_t fader = 0;
    uint8_t t = 0;
    rgb_s tmp_color = {.rgb=current_color.rgb};
    
    for (uint8_t i = 0; i < 2; i++)
    {
        next_color.rgb = color.rgb;
        fader = 0;
        t = 0;
        while(fader < 30)
        {
            t = 0;
            if ((8 * fader) > 255)
            {
                t = 255;
            }
            else
            {
                t = (8 * fader);
            }
            rgb_blend(&current_color, last_color, next_color, t);
            rgb_setall(current_color);
            rgb_show();
            fader += 1;
            vTaskDelay(5/portTICK_PERIOD_MS);
        }
        last_color.rgb = next_color.rgb;
        rgb_setall(next_color);
        rgb_show();

        next_color.rgb = tmp_color.rgb;
        fader = 0;
        t = 0;
        while(fader < 30)
        {
            t = 0;
            if ((8 * fader) > 255)
            {
                t = 255;
            }
            else
            {
                t = (8 * fader);
            }
            rgb_blend(&current_color, last_color, next_color, t);
            rgb_setall(current_color);
            rgb_show();
            fader += 1;
            vTaskDelay(5/portTICK_PERIOD_MS);
        }

        last_color.rgb = tmp_color.rgb;
        rgb_setall(tmp_color);
        rgb_show();
    }

    rgb_blinking = false;
}

void rgb_animator_task(void * param)
{
    rgb_msg_s anim_msg = {0};

    for(;;)
    {
        if (xQueueReceive(rgb_animator_Queue, &(anim_msg), (TickType_t) 0))
        {
            switch(anim_msg.msg)
            {
                default:
                case RGB_MSG_FADE:
                    rgb_animate_internal_fade(anim_msg.color);
                    break;
                
                case RGB_MSG_BLINK:
                    rgb_animate_internal_blink(anim_msg.color);
                    break;
            }
            
        }
        vTaskDelay(32/portTICK_PERIOD_MS);
    }
}

void rgb_animate_to(rgb_s color)
{
    if (color.rgb == rgb_msg_fade.color.rgb)
    {
        return;
    }

    rgb_msg_fade.color.rgb = color.rgb;
    rgb_msg_fade.msg = RGB_MSG_FADE;

    xQueueSend(rgb_animator_Queue, &rgb_msg_fade, (TickType_t) 0);
}

void rgb_animate_blink(rgb_s color)
{
    rgb_blinking = true;

    rgb_msg_blink.color.rgb = color.rgb;
    rgb_msg_blink.msg = RGB_MSG_BLINK;

    xQueueSend(rgb_animator_Queue, &rgb_msg_blink, (TickType_t) 0);
}


void rgb_animator_init()
{
    rgb_animator_Queue = xQueueCreate(10, sizeof(rgb_msg_s));
    xTaskCreatePinnedToCore(rgb_animator_task, "rgb_task", 2048, NULL, 3, &rgb_animator_TaskHandle, 1);
}


