#ifndef UTIL_RGB_H
#define UTIL_RGB_H

#include "adapter_includes.h"
#include "driver/spi_master.h"
#include "hal/spi_hal.h"
#include <math.h>

// Defines for configuration of RGB driver utility
#define RGB_SPIBUS  SPI2_HOST
#define GC_LED_COUNT 1
#define GC_LED_GPIO GPIO_NUM_38

#define RGB_MODE_RGB    0
#define RGB_MODE_GRB    1

#define RGB_T0H     1
#define RGB_T0L     2
#define RGB_T1H     2
#define RGB_T1L     1

#define RGB_HIGH    0x6
#define RGB_LOW     0x4

// How many bytes per LED?
#define RGB_BYTE_MULTIPLIER 9
// How many bytes per color?
#define RGB_BYTE_COLOR      3

typedef struct
{
    union
    {
        struct
        {
            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t dummy;
        };
        uint32_t rgb;
    };
    
} rgb_s;

// We use this type to set the bits
// appropriately for the SPI output.
// Easier to work with one 32 bit int
// instead of each individual byte.
// 3 bytes are allocated to each color
typedef struct 
{
    union
    {
        struct 
        {
            uint8_t byte2;
            uint8_t byte1;
            uint8_t byte0;
            uint8_t dummy;
        };
        uint32_t splitter; 
    };
} rgb_splitter_s;

typedef struct
{
    spi_device_handle_t *spi_handle;
    rgb_s colors1[GC_LED_COUNT];
    rgb_s colors2[GC_LED_COUNT];
    uint16_t frames;
} rgb_anim_s;

typedef enum
{
    UTIL_RGB_STATUS_AVAILABLE = 0,
    UTIL_RGB_STATUS_DISABLED  = 1,
} util_rgb_status_t;

extern util_rgb_status_t util_rgb_status;

typedef enum
{
    UTIL_RGB_MODE_RGB = 0,
    UTIL_RGB_MODE_GRB = 1,
} util_rgb_mode_t;

extern util_rgb_mode_t util_rgb_mode;

#define COLOR_BLACK     (rgb_s) {.rgb = 0x000000}
#define COLOR_RED       (rgb_s) {.rgb = 0xFF0000}
#define COLOR_ORANGE    (rgb_s) {.rgb = 0xFF8000}
#define COLOR_YELLOW    (rgb_s) {.rgb = 0xFFFF00}
#define COLOR_GREEN     (rgb_s) {.rgb = 0x00FF00}
#define COLOR_TEAL      (rgb_s) {.rgb = 0x00FF80}
#define COLOR_CYAN      (rgb_s) {.rgb = 0x00FFFF}
#define COLOR_BLUE      (rgb_s) {.rgb = 0x0000FF}
#define COLOR_VIOLET    (rgb_s) {.rgb = 0x8000FF}
#define COLOR_PURPLE    (rgb_s) {.rgb = 0xFF00FF}

// Blend two colors together.
// color1 - The first color
// color2 - The second color
// blend_amount - The amount to blend the two colors. Between 0 and 255.
// Returns an rgb_s color type.
rgb_s rgb_blend(rgb_s color1, rgb_s color2, uint8_t blend_amount);

rgb_s rgb_from_hsv(uint8_t h, int8_t s, uint8_t v);

void rgb_setbrightness(uint8_t brightness);

// Set all RGB colors to the same.
void rgb_setall(rgb_s color, rgb_s *led_colors);

// Update the LED colors and brightness.
void rgb_show();

// Initialize RGB Controller Backend
// Only support WS2812B Protocol!
esp_err_t util_rgb_init(rgb_s *led_colors, util_rgb_mode_t mode);

#endif