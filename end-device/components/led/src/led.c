#include "esp_err.h"
#include "esp_log.h"
#include "led_strip.h"
#include "led_strip_types.h"
#include "led_strip_rmt.h"

#define TAG "LED"

static led_strip_handle_t led_strip;

void set_led_color(int r, int g, int b)
{
    if(r >= 0 && r < 256 && g >= 0 && g < 256 && b >= 0 && b < 256){
        led_strip_set_pixel(led_strip, 0, r, g, b);
        led_strip_refresh(led_strip);
    }
}

void init_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}