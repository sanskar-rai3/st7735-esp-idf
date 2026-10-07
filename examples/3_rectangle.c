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
    ESP_ERROR_CHECK(st7735_draw_clear(ST7735_BLACK));

    ESP_ERROR_CHECK(st7735_draw_rect(10, 10, 80, 50, ST7735_RED));
    ESP_ERROR_CHECK(st7735_draw_rect(20, 20, 60, 30, ST7735_GREEN));

    ESP_ERROR_CHECK(st7735_draw_rect_fill(
        30, 70, 50, 40, ST7735_BLUE
    ));

    ESP_ERROR_CHECK(st7735_draw_rect_fill(
        90, 20, 30, 30, ST7735_YELLOW
    ));

    while (1)
        vTaskDelay(pdMS_TO_TICKS(1000));
}
