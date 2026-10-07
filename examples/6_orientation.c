#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7735.h"

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

    ESP_ERROR_CHECK(st7735_set_orientation(
        ST7735_ORIENTATION_90
    ));

    ESP_ERROR_CHECK(st7735_draw_clear(ST7735_WHITE));

    int w = st7735_get_width();
    int h = st7735_get_height();

    ESP_ERROR_CHECK(st7735_draw_rect(0, 0, w, h, ST7735_RED));

    ESP_ERROR_CHECK(st7735_draw_line(0, 0, w - 1, h - 1, ST7735_BLUE));

    ESP_ERROR_CHECK(st7735_draw_text(10, 10, "90 DEG", ST7735_BLACK, ST7735_WHITE, 2));

    while (1)
        vTaskDelay(pdMS_TO_TICKS(1000));
}
