#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7735.h"

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
    ESP_ERROR_CHECK(st7735_draw_clear(ST7735_RED));

    while (1)
        vTaskDelay(pdMS_TO_TICKS(1000));
}
