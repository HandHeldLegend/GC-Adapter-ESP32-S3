#include "main.h"

neo_rgb_s _colors[CONFIG_NP_RGB_COUNT] = {0};

void rgb_itf_update(rgb_s *leds)
{
    for(uint8_t i = 0; i < CONFIG_NP_RGB_COUNT; i++)
    {
        _colors[i].red      = leds[i].r;
        _colors[i].green    = leds[i].g;
        _colors[i].blue     = leds[i].b;
    }
    neo_rgb_show();
}

void rgb_itf_init()
{
    neopixel_init(_colors, SPI3_HOST);
    neo_rgb_setbrightness(200);
}