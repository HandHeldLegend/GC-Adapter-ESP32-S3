#ifndef RGB_ANIMATOR_H
#define RGB_ANIMATOR_H

#include "adapter_includes.h"

typedef enum
{
    RGB_MSG_FADE = 0,
    RGB_MSG_BLINK,
} rgb_msg_t;

typedef struct
{
    rgb_s color;
    rgb_msg_t msg;
} rgb_msg_s;

extern rgb_s mode_color;

void rgb_animate_to(rgb_s color);
void rgb_animate_blink(rgb_s color);
void rgb_animator_init();

#endif
