#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"

/**
 * Colors are stored as 16 bit unsigned integers with RGB565 format
 * 
 * Red   - 5 bits
 * Green - 6 bits
 * Blue  - 5 bits
 *
 * Green gets the extra bit since human eyes are more sensitive to green
 */

/* Colors */
#define ST7735_WHITE    0xFFFFu
#define ST7735_BLACK    0x0000u
#define ST7735_RED      0xF800u
#define ST7735_GREEN    0x07E0u
#define ST7735_BLUE     0x001Fu
#define ST7735_YELLOW   0xFFE0u
#define ST7735_CYAN     0x07FFu
#define ST7735_MAGENTA  0xF81Fu

/**
 * @brief The hardware configuration of the display
 */
typedef struct {
    spi_host_device_t host;

    gpio_num_t sclk;
    gpio_num_t mosi;
    gpio_num_t cs;

    gpio_num_t dc;
    gpio_num_t rst;
} ST7735_Config;

/**
 * @brief Initialize st7735 library
 *
 * @param config Hardware configuration for the display
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_init(const ST7735_Config *config);

/**
 * @brief Fills the entire display with the specified color.
 *
 * @param color Color used to fill the display.
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_draw_clear(uint16_t color);

/**
 * @brief Draws a single pixel at (x, y)
 *
 * @param x The x-coordinate of the screen
 * @param y The y-coordinate of the screen
 *
 * @param color Color of the pixel
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_draw_pixel(int x, int y, uint16_t color);

/**
 * @brief Draws a line from (x1, y1) to (x2, y2)
 *
 * @param x1 The x-coordinate of the first position
 * @param y1 The y-coordinate of the first position
 *
 * @param x2 The x-coordinate of the second position
 * @param y2 The y-coordinate of the second position
 *
 * @param color Color of the line
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_draw_line(int x1, int y1, int x2, int y2, uint16_t color);

/**
 * @brief Draws a hollow rectangle of size w * h with its top-left vertex at (x, y)
 *
 * @param x The x-coordinate of the screen
 * @param y The y-coordinate of the screen
 *
 * @param w Width of the rectangle
 * @param h Height of the rectangle
 *
 * @param color The color of the rectangle
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_draw_rect(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Draws a filled rectangle of size w * h with its top-left vertex at (x, y)
 *
 * @param x The x-coordinate of the screen
 * @param y The y-coordinate of the screen
 *
 * @param w Width of the rectangle
 * @param h Height of the rectangle
 *
 * @param color Color of the rectangle
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_draw_rect_fill(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Draws a hollow circle of radius r with its center at (x, y)
 *
 * @param x The x-coordinate of the screen
 * @param y The y-coordinate of the screen
 *
 * @param r Radius of the circle
 *
 * @param color Color of the Circle
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_draw_circle(int x, int y, int r, uint16_t color);

/**
 * @brief Draws a filled circle of radius r with its center at (x, y)
 *
 * @param x The x-coordinate of the screen
 * @param y The y-coordinate of the screen
 *
 * @param r Radius of the circle
 *
 * @param color Color of the Circle
 *
 * @return ESP_OK on sucess, corresponding error code on failure
 */
esp_err_t st7735_draw_circle_fill(int x, int y, int r, uint16_t color);

/**
 * @brief Draws a character with its top-left corner at (x, y)
 *
 * @param x The x-coordinate of the screen
 * @param y The y-coordinate of the screen
 *
 * @param c Character to draw
 *
 * @param color Color of the char
 *
 * @param scale Scale factor of the text
 *
 * @return ESP_OK on success, corresponding error code on failure
 */
esp_err_t st7735_draw_char(int x, int y, char c, uint16_t color, int scale);

/**
 * @brief Draws text with its top-left corner at (x, y)
 *
 * @param x The x-coordinate of the screen
 * @param y The y-coordinate of the screen
 *
 * @param txt Null-terminated string to draw
 *
 * @param color Color of the text
 *
 * @param scale Scale factor of the text
 *
 * @return ESP_OK on success, corresponding error code on failure
 */
esp_err_t st7735_draw_text(int x, int y, const char *txt, uint16_t color, int scale);

#endif /* ST7735_H */
