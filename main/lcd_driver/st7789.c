
#include "st7789.h"

static const char *TAG_LCD = "LCD";

esp_lcd_panel_handle_t panel_handle = NULL;

void lcd_init(void) {
    ESP_LOGI(TAG_LCD, "Install panel IO");

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = CUSTOM_PIN_NUM_LCD_DC,
        .cs_gpio_num = CUSTOM_PIN_NUM_LCD_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG_LCD, "Install ST7789 panel driver");

    esp_lcd_panel_dev_st7789t_config_t panel_config = {
        .reset_gpio_num = CUSTOM_PIN_NUM_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789t(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // Set gap/offsets
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, Offset_X, Offset_Y));

    ESP_LOGI(TAG_LCD, "LCD initialized");
}

void backlight_brightness(uint8_t brightness_percent) {
    if (brightness_percent > 100) brightness_percent = 100;

    ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_LS_MODE,
                                      .timer_num = LEDC_HS_TIMER,
                                      .duty_resolution = LEDC_ResolutionRatio,
                                      .freq_hz = 5000,
                                      .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_LS_MODE,
                                          .channel = LEDC_HS_CH0_CHANNEL,
                                          .timer_sel = LEDC_HS_TIMER,
                                          .intr_type = LEDC_INTR_DISABLE,
                                          .gpio_num = LEDC_HS_CH0_GPIO,
                                          .duty = 0,
                                          .hpoint = 0};
    ledc_channel_config(&ledc_channel);

    uint32_t duty = (LEDC_MAX_Duty * brightness_percent) / 100;
    ledc_set_duty(LEDC_LS_MODE, LEDC_HS_CH0_CHANNEL, duty);
    ledc_update_duty(LEDC_LS_MODE, LEDC_HS_CH0_CHANNEL);
}
