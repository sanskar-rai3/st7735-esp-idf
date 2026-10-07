#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7735.h"

#define WINDOW_W 40
#define WINDOW_H 30

static const uint16_t pixels[WINDOW_W * WINDOW_H] = {
    ...
};

void app_main(void) {
    ST7735_Config config = {
        .host = SPI2_HOST,
        .sclk = GPIO_NUM_18,
        .mosi = GPIO_NUM_23,
        .cs   = GPIO_NUM_5,
        .dc   = GPIO_NUM_2,
        .rst  = GPIO_NUM_4,
    };

    ESP_ERROR_CHECK(st7735_init(&config));
    ESP_ERROR_CHECK(st7735_draw_clear(ST7735_BLACK));

    ESP_ERROR_CHECK(st7735_set_window(20, 20, 20 + WINDOW_W - 1, 20 + WINDOW_H - 1));

    ESP_ERROR_CHECK(st7735_write_pixels(pixels, WINDOW_W * WINDOW_H));

    while (1)
        vTaskDelay(pdMS_TO_TICKS(1000));
}
