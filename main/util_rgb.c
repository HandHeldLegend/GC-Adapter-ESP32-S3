#include "util_rgb.h"

spi_device_handle_t rgb_spi = NULL;
util_rgb_status_t util_rgb_status = UTIL_RGB_STATUS_DISABLED;
util_rgb_mode_t util_rgb_mode = UTIL_RGB_MODE_RGB;
rgb_s *current_colors = NULL;
uint8_t util_rgb_brightness = 128;

rgb_s rgb_blend(rgb_s color1, rgb_s color2, uint8_t blend_amount)
{
    float ratio = blend_amount/255;
    int rdif = abs((color1.red + color2.red))    /2;
    int gdif = abs((color1.green + color2.green))/2;
    int bdif = abs((color1.blue + color2.blue))  /2;

    rdif = rdif * ratio;
    gdif = gdif * ratio;
    bdif = bdif * ratio;

    rgb_s output = {
        .red = color1.red + (uint8_t) rdif,
        .green = color1.green + (uint8_t) gdif,
        .blue = color1.blue + (uint8_t) bdif,
    };

    return output;
}

#define HSV_SECTION_6 (0x20)
#define HSV_SECTION_3 (0x40)

// Function taken from https://github.com/FastLED/FastLED/blob/master/src/hsv2rgb.cpp
rgb_s rgb_from_hsv(uint8_t h, int8_t s, uint8_t v)
{
    rgb_s color_out = {
        .red = 0x00,
        .green = 0x00,
        .blue = 0x00,
    };

    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = v;
    uint8_t saturation = s;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = 255 - saturation;
    uint8_t brightness_floor = (value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = h / HSV_SECTION_3; // 0..2
    uint8_t offset = h % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


    if( section ) {
        if( section == 1) {
            // section 1: 0x40..0x7F
            color_out.red = brightness_floor;
            color_out.green = rampdown_adj_with_floor;
            color_out.blue = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            color_out.red = rampup_adj_with_floor;
            color_out.green = brightness_floor;
            color_out.blue = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        color_out.red = rampdown_adj_with_floor;
        color_out.green = rampup_adj_with_floor;
        color_out.blue = brightness_floor;
    }

    return color_out;
}

void rgb_create_packet(uint8_t *buffer)
{
    const char* TAG = "rgb_create_packet";
    // Clear the buffer
    memset(buffer, 0x0, GC_LED_COUNT*RGB_BYTE_MULTIPLIER);
    uint8_t buffer_idx = 0;

    rgb_s adjusted_colors[GC_LED_COUNT] = {0x00};

    memcpy(adjusted_colors, current_colors, sizeof(rgb_s) * GC_LED_COUNT);

    float ratio = (float) util_rgb_brightness / 255.0;
    for (uint8_t s = 0; s < GC_LED_COUNT; s++)
    {
        float newRed    = (float) adjusted_colors[s].red     * ratio;
        float newGreen  = (float) adjusted_colors[s].green   * ratio;
        float newBlue   = (float) adjusted_colors[s].blue    * ratio;
        adjusted_colors[s].red = (uint8_t) newRed;
        adjusted_colors[s].green = (uint8_t) newGreen;
        adjusted_colors[s].blue = (uint8_t) newBlue;
    }

    // Set up a splitter
    rgb_splitter_s s_red    = {0};
    rgb_splitter_s s_green  = {0};
    rgb_splitter_s s_blue   = {0};

    // Cycle through each LED first
    for(uint8_t i = 0; i < GC_LED_COUNT; i++)
    {
        // There are three bytes to contend with
        // for each LED. Each color is split up to occupy three SPI bytes.
        // We have to cycle through each bit of each color and shift in the
        // appropriate data.
        memset(&s_red, 0, sizeof(rgb_splitter_s));
        memset(&s_green, 0, sizeof(rgb_splitter_s));
        memset(&s_blue, 0, sizeof(rgb_splitter_s));
        
        // Keep track of which bit
        // we are setting with an index.
        uint8_t s_idx   = 0;

        for(uint8_t b = 0; b < 8; b++)
        {
            uint8_t red_bit = 0;
            uint8_t green_bit = 0;
            uint8_t blue_bit = 0;

            // Account for GRB mode (From rightmost bit to left)
            if (util_rgb_mode == UTIL_RGB_MODE_GRB)
            {
                red_bit     = (adjusted_colors[i].green  >> (7-b)) & 1;
                green_bit   = (adjusted_colors[i].red    >> (7-b)) & 1;
            }
            else
            {
                red_bit     = (adjusted_colors[i].red    >> (7-b)) & 1;
                green_bit   = (adjusted_colors[i].green  >> (7-b)) & 1;
            }
            
            blue_bit   =    (adjusted_colors[i].blue  >> (7-b)) & 1;
            
            // Set the bits in the splitter from least significant to most.
            if (red_bit)
            {
                s_red.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_red.splitter      |= (RGB_LOW << (s_idx));
            }

            if (green_bit)
            {
                s_green.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_green.splitter      |= (RGB_LOW << (s_idx));
            }

            if (blue_bit)
            {
                s_blue.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_blue.splitter      |= (RGB_LOW << (s_idx));
            }

            s_idx   += 3;
        }

        // On ESP32-S3 we have to invert the byte : )
        #if CONFIG_IDF_TARGET_ESP32S3
            // Once we've processed all 8 bits of the three colors, copy to our SPI buffer
            buffer[buffer_idx]  = ~s_red.byte0;
            buffer[buffer_idx+1] = ~s_red.byte1;
            buffer[buffer_idx+2] = ~s_red.byte2;

            buffer[buffer_idx+3] = ~s_green.byte0;
            buffer[buffer_idx+4] = ~s_green.byte1;
            buffer[buffer_idx+5] = ~s_green.byte2;

            buffer[buffer_idx+6] = ~s_blue.byte0;
            buffer[buffer_idx+7] = ~s_blue.byte1;
            buffer[buffer_idx+8] = ~s_blue.byte2;
        #else
            buffer[buffer_idx] = s_red.byte0;
            buffer[buffer_idx+1] = s_red.byte1;
            buffer[buffer_idx+2] = s_red.byte2;

            buffer[buffer_idx+3] = s_green.byte0;
            buffer[buffer_idx+4] = s_green.byte1;
            buffer[buffer_idx+5] = s_green.byte2;

            buffer[buffer_idx+6] = s_blue.byte0;
            buffer[buffer_idx+7] = s_blue.byte1;
            buffer[buffer_idx+8] = s_blue.byte2;
        #endif

        // Increase our buffer idx
        buffer_idx += 9;
    }

}

void rgb_setbrightness(uint8_t brightness)
{
    util_rgb_brightness = brightness;
}

void rgb_setall(rgb_s color, rgb_s *led_colors)
{
    const char* TAG = "rgb_setall";
    
    for (uint8_t i = 0; i < GC_LED_COUNT; i++)
    {
        led_colors[i].rgb = color.rgb;
    }
}

void rgb_show()
{   
    
    if(util_rgb_status != UTIL_RGB_STATUS_AVAILABLE)
    {
        const char* TAG = "rgb_show";
        ESP_LOGE(TAG, "RGB Utility not initialized. Initialize before using this.");
        return;
    }

    uint8_t rgb_spi_buffer[GC_LED_COUNT*RGB_BYTE_MULTIPLIER];
    rgb_create_packet(rgb_spi_buffer);

    spi_transaction_t trans = {
        .length = GC_LED_COUNT*RGB_BYTE_MULTIPLIER*8,
        .tx_buffer = rgb_spi_buffer,
        .user=(void*)1,
    };

    esp_err_t err = spi_device_transmit(rgb_spi, &trans);
}

esp_err_t util_rgb_init(rgb_s *led_colors, util_rgb_mode_t mode)
{
    const char* TAG = "util_rgb_init";
    esp_err_t err; 

    // Set up SPI for rgb
    
    // Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num    = (int) GC_LED_GPIO,
        .miso_io_num    = -1,
        .sclk_io_num    = -1,
        .quadwp_io_num  = -1,
        .quadhd_io_num  = -1,
    };

    // Configuration for the SPI master interface
    spi_device_interface_config_t devcfg={
        .mode           = 0,
        .clock_speed_hz = 2500000, //2.5Mhz
        .spics_io_num   = -1,
        .queue_size     = 7,
        .input_delay_ns = 0,
    };

    err = spi_bus_initialize(RGB_SPIBUS, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI Bus.");
        util_rgb_status = UTIL_RGB_STATUS_DISABLED;
        return err;
    }

    err = spi_bus_add_device(RGB_SPIBUS, &devcfg, &rgb_spi);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add SPI Device.");
        util_rgb_status = UTIL_RGB_STATUS_DISABLED;
        return err;
    }

    // Have to invert because some dumb reason on ESP32-S3 I don't understand : )
    #if CONFIG_IDF_TARGET_ESP32S3
        gpio_matrix_out(GC_LED_GPIO, FSPID_OUT_IDX, true, false);
    #endif

    ESP_LOGI(TAG, "Started RGB Service OK.");
    util_rgb_status = UTIL_RGB_STATUS_AVAILABLE;
    util_rgb_mode = mode;
    // Set current colors to the pointer referenced by user.
    current_colors = led_colors;

    return ESP_OK;
}