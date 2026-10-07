#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7735.h"

#define BITMAP_W 120
#define BITMAP_H 160

static const uint16_t bitmap[BITMAP_W * BITMAP_H] = {
    // use img2bitmap.py to generate bitmap
};

void app_main(void)
{
    ST7735_Config config = {
        .host = SPI2_HOST,
        .sclk = GPIO_NUM_18,
        .mosi = GPIO_NUM_23,
        .cs   = GPIO_NUM_5,
        .dc   = GPIO_NUM_2,
        .rst  = GPIO_NUM_4,
    };

    ESP_ERROR_CHECK(st7735_init(&config));
    ESP_ERROR_CHECK(st7735_draw_clear(ST7735_WHITE));

    ESP_ERROR_CHECK(st7735_draw_bitmap(0, 0, BITMAP_W, BITMAP_H, bitmap));

    while (1)
        vTaskDelay(pdMS_TO_TICKS(1000));
}
