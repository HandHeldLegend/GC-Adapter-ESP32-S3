#include "rgb_animator.h"

rgb_s last_color = {0};
rgb_s current_color = {0};
rgb_s next_color = {0};

void rgb_animate_to(rgb_s color)
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


