#include "st7735.h"
#include "font.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"

/*  Enable debug mode */
// #define ST7735_DEBUG

/* ST7735 Commands */
#define ST7735_NOP        0x00u
#define ST7735_SWRESET    0x01u
#define ST7735_RDDID      0x04u
#define ST7735_RDDST      0x09u
#define ST7735_RDDPM      0x0Au
#define ST7735_RDDMADCTL  0x0Bu
#define ST7735_RDDCOLMOD  0x0Cu
#define ST7735_RDDIM      0x0Du
#define ST7735_RDDSDR     0x0Fu

#define ST7735_SLPIN      0x10u
#define ST7735_SLPOUT     0x11u
#define ST7735_PTLON      0x12u
#define ST7735_NORON      0x13u

#define ST7735_INVOFF     0x20u
#define ST7735_INVON      0x21u
#define ST7735_GAMSET     0x26u
#define ST7735_DISPOFF    0x28u
#define ST7735_DISPON     0x29u
#define ST7735_CASET      0x2Au
#define ST7735_RASET      0x2Bu
#define ST7735_RAMWR      0x2Cu
#define ST7735_RGBSET     0x2Du

#define ST7735_PTLAR      0x30u
#define ST7735_TEOFF      0x34u
#define ST7735_TEON       0x35u
#define ST7735_MADCTL     0x36u
#define ST7735_IDMOFF     0x38u
#define ST7735_IDMON      0x39u
#define ST7735_COLMOD     0x3Au

#define ST7735_FRMCTR1    0xB1u
#define ST7735_INVCTR     0xB4u

#define ST7735_PWCTR1     0xC0u
#define ST7735_PWCTR2     0xC1u
#define ST7735_PWCTR3     0xC2u
#define ST7735_PWCTR4     0xC3u
#define ST7735_PWCTR5     0xC4u
#define ST7735_VMCTR1     0xC5u

#define ST7735_RDID1      0xDAu
#define ST7735_RDID2      0xDBu
#define ST7735_RDID3      0xDCu
#define ST7735_RDID4      0xDDu

#define ST7735_GMCTRP1    0xE0u
#define ST7735_GMCTRN1    0xE1u

#define ST7735_CLAMP(value, min, max) \
    ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

static int g_st7735_width  = 128;
static int g_st7735_height = 160;

static ST7735_Config g_config;
static spi_device_handle_t g_spi;

static esp_err_t spi_init(void) {
    spi_bus_config_t bus_config = {
        .mosi_io_num = g_config.mosi,
        .miso_io_num = -1,
        .sclk_io_num = g_config.sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    esp_err_t err = spi_bus_initialize(g_config.host, &bus_config, SPI_DMA_CH_AUTO);

    if (err != ESP_OK)
        return err;

    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 20 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = g_config.cs,
        .queue_size = 1,
    };

    return spi_bus_add_device(g_config.host, &device_config, &g_spi);
}

static esp_err_t gpio_init(void) {
    gpio_config_t gpio = {
        .pin_bit_mask = (1ULL << g_config.dc)  |
                        (1ULL << g_config.rst),

        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    return gpio_config(&gpio);
}

static void tft_reset(void) {
    gpio_set_level(g_config.rst, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    gpio_set_level(g_config.rst, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
}

static esp_err_t tft_write_command(uint8_t command) {
    gpio_set_level(g_config.dc, 0);

    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &command,
    };

    return spi_device_transmit(g_spi, &transaction);
}

static esp_err_t tft_write_data(const uint8_t *data, size_t len) {
    gpio_set_level(g_config.dc, 1);

    spi_transaction_t transaction = {
        .length = len * 8,
        .tx_buffer = data,
    };

    return spi_device_transmit(g_spi, &transaction);
}

static esp_err_t tft_init_sequence(void) {
    esp_err_t err;

    /* Software reset */
    err = tft_write_command(ST7735_SWRESET);
    if (err != ESP_OK)
        return err;

    vTaskDelay(pdMS_TO_TICKS(150));

    /* Sleep out */
    err = tft_write_command(ST7735_SLPOUT);
    if (err != ESP_OK)
        return err;

    vTaskDelay(pdMS_TO_TICKS(500));

    /* Frame rate control */
    {
        const uint8_t data[] = {
            0x01, 0x2C, 0x2D
        };

        err = tft_write_command(ST7735_FRMCTR1);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Display inversion control */
    {
        const uint8_t data[] = {
            0x07
        };

        err = tft_write_command(ST7735_INVCTR);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Power control 1 */
    {
        const uint8_t data[] = {
            0xA2, 0x02, 0x84
        };

        err = tft_write_command(ST7735_PWCTR1);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Power control 2 */
    {
        const uint8_t data[] = {
            0xC5
        };

        err = tft_write_command(ST7735_PWCTR2);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Power control 3 */
    {
        const uint8_t data[] = {
            0x0A, 0x00
        };

        err = tft_write_command(ST7735_PWCTR3);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Power control 4 */
    {
        const uint8_t data[] = {
            0x8A, 0x2A
        };

        err = tft_write_command(ST7735_PWCTR4);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* VCOM control */
    {
        const uint8_t data[] = {
            0x0E
        };

        err = tft_write_command(ST7735_VMCTR1);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Memory access control */
    {
        const uint8_t data[] = {
            0xC0
        };

        err = tft_write_command(ST7735_MADCTL);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Pixel format: 16-bit RGB565 */
    {
        const uint8_t data[] = {
            0x05
        };

        err = tft_write_command(ST7735_COLMOD);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Positive gamma correction */
    {
        const uint8_t data[] = {
            0x02, 0x1C, 0x07, 0x12,
            0x37, 0x32, 0x29, 0x2D,
            0x29, 0x25, 0x2B, 0x39,
            0x00, 0x01, 0x03, 0x10
        };

        err = tft_write_command(ST7735_GMCTRP1);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Negative gamma correction */
    {
        const uint8_t data[] = {
            0x03, 0x1D, 0x07, 0x06,
            0x2E, 0x2C, 0x29, 0x2D,
            0x2E, 0x2E, 0x37, 0x3F,
            0x00, 0x00, 0x02, 0x10
        };

        err = tft_write_command(ST7735_GMCTRN1);
        if (err != ESP_OK)
            return err;

        err = tft_write_data(data, sizeof(data));
        if (err != ESP_OK)
            return err;
    }

    /* Normal display mode */
    err = tft_write_command(ST7735_NORON);
    if (err != ESP_OK)
        return err;

    vTaskDelay(pdMS_TO_TICKS(10));

    /* Display on */
    err = tft_write_command(ST7735_DISPON);
    if (err != ESP_OK)
        return err;

    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

esp_err_t st7735_init(const ST7735_Config *config) {
    esp_err_t err;

    if (config == NULL)
        return ESP_ERR_INVALID_ARG;

    g_config = *config;

    /* Initialize control GPIOs */
    err = gpio_init();
    if (err != ESP_OK)
        return err;

    /* Initialize SPI */
    err = spi_init();
    if (err != ESP_OK)
        return err;

    /* Hardware reset */
    tft_reset();

    /* Initialize ST7735 controller */
    err = tft_init_sequence();
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}


static esp_err_t tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    esp_err_t err;
    uint8_t data[4];

    if (x0 > x1 || y0 > y1)
        return ESP_ERR_INVALID_ARG;

    if (x1 >= g_st7735_width || y1 >= g_st7735_height)
        return ESP_ERR_INVALID_ARG;

    /* Set column address */
    data[0] = x0 >> 8;
    data[1] = x0 &  0xFF;

    data[2] = x1 >> 8;
    data[3] = x1 &  0xFF;

    err = tft_write_command(ST7735_CASET);
    if (err != ESP_OK)
        return err;

    err = tft_write_data(data, sizeof(data));
    if (err != ESP_OK)
        return err;

    /* Set row address */
    data[0] = y0 >> 8;
    data[1] = y0 &  0xFF;

    data[2] = y1 >> 8;
    data[3] = y1 &  0xFF;

    err = tft_write_command(ST7735_RASET);
    if (err != ESP_OK)
        return err;

    err = tft_write_data(data, sizeof(data));
    if (err != ESP_OK)
        return err;

    /* Start writing to display RAM */
    return tft_write_command(ST7735_RAMWR);
}

#define ST7735_TRANSFER_PIXELS 256

esp_err_t st7735_draw_clear(uint16_t color) {
    esp_err_t err;

    err = tft_set_window(0, 0, g_st7735_width - 1, g_st7735_height - 1);
    if (err != ESP_OK)
        return err;

    uint8_t buffer[ST7735_TRANSFER_PIXELS * 2];

    for (size_t i = 0; i < ST7735_TRANSFER_PIXELS; i++) {
        buffer[i * 2]     = color >> 8;
        buffer[i * 2 + 1] = color &  0xFF;
    }

    size_t remaining = g_st7735_width * g_st7735_height;

    while (remaining > 0) {
        size_t pixels = remaining;

        if (pixels > ST7735_TRANSFER_PIXELS)
            pixels = ST7735_TRANSFER_PIXELS;

        err = tft_write_data(buffer, pixels * 2);
        if (err != ESP_OK)
            return err;

        remaining -= pixels;
    }

    return ESP_OK;
}

esp_err_t st7735_set_orientation(ST7735_Orientation orientation) {
    uint8_t madctl;
    int width;
    int height;

    switch (orientation) {
        case ST7735_ORIENTATION_0:
            madctl = 0x00;
            width = 128;
            height = 160;
            break;

        case ST7735_ORIENTATION_90:
            madctl = 0x60;
            width = 160;
            height = 128;
            break;

        case ST7735_ORIENTATION_180:
            madctl = 0xC0;
            width = 128;
            height = 160;
            break;

        case ST7735_ORIENTATION_270:
            madctl = 0xA0;
            width = 160;
            height = 128;
            break;

        default:
            return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = tft_write_command(ST7735_MADCTL);
    if (err != ESP_OK)
        return err;

    err = tft_write_data(&madctl, 1);
    if (err != ESP_OK)
        return err;

    g_st7735_width = width;
    g_st7735_height = height;

    return ESP_OK;
}

esp_err_t st7735_draw_pixel(int x, int y, uint16_t color) {
    esp_err_t err;

    err = tft_set_window(x, y, x, y);
    if (err != ESP_OK)
        return err;
    
    uint8_t buffer[2] = {
        color >> 8,
        color &  0xFF
    };

    err = tft_write_data(buffer, 2);
    if (err != ESP_OK)
        return err;
    
    return ESP_OK;
}

esp_err_t st7735_draw_char(int x, int y, char c, uint16_t fg_color, uint16_t bg_color, int scale) {
    if (c < 0x20 || c > 0x7E)
        return ESP_ERR_INVALID_ARG;

    scale = ST7735_CLAMP(scale, 1, FONT_MAX_SCALE);

#ifdef ST7735_DEBUG
    ESP_LOGI("ST7735", "draw_char: c='%c' 0x%02X scale=%d", c, (unsigned char)c, scale);
#endif

    const uint8_t *glyph = &font[(c - 0x20) * FONT_HEIGHT];

    const int width  = FONT_WIDTH * scale;
    const int height = FONT_HEIGHT * scale;

    uint8_t buffer[width * height * 2];

    for (size_t i = 0; i < width * height; i++) {
        buffer[i * 2]     = bg_color >> 8;
        buffer[i * 2 + 1] = bg_color & 0xFF;
    }

    for (int row = 0; row < FONT_HEIGHT; row++) {
        for (int col = 0; col < FONT_WIDTH; col++) {
            if (!(glyph[row] & (1 << (FONT_WIDTH - 1 - col))))
                continue;

            for (int dy = 0; dy < scale; dy++) {
                for (int dx = 0; dx < scale; dx++) {
                    int px = col * scale + dx;
                    int py = row * scale + dy;

                    size_t index = (py * width + px) * 2;

                    buffer[index]     = fg_color >> 8;
                    buffer[index + 1] = fg_color & 0xFF;
                }
            }
        }
    }

    esp_err_t err = tft_set_window(x, y, x + width - 1, y + height - 1);

    if (err != ESP_OK)
        return err;

    return tft_write_data(buffer, width * height * 2);
}

esp_err_t st7735_draw_text(int x, int y, const char *txt, uint16_t fg_color, uint16_t bg_color, int scale) {
    if (txt == NULL)
        return ESP_ERR_INVALID_ARG;

    scale = ST7735_CLAMP(scale, 1, FONT_MAX_SCALE);

#ifdef ST7735_DEBUG
    ESP_LOGI("ST7735", "draw_text: \"%s\" scale=%d", txt, scale);
#endif

    const int char_width = FONT_WIDTH * scale;

    while (*txt) {
        if (x + char_width > g_st7735_width)
            break;

        esp_err_t err = st7735_draw_char(x, y, *txt, fg_color, bg_color, scale);

        if (err != ESP_OK)
            return err;

        x += char_width;
        txt++;
    }

    return ESP_OK;
}
