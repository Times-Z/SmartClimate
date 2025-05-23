#pragma once
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/ledc.h>

#include "lvgl.h"
#include "vernon_st7789t.h"
#include "lvgl_driver.h"

// LCD SPI GPIO
// Using SPI2
#define LCD_HOST SPI2_HOST

#define LCD_PIXEL_CLOCK_HZ (12 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL 1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL
#define CUSTOM_PIN_NUM_SCLK 7
#define CUSTOM_PIN_NUM_MOSI 6
#define CUSTOM_PIN_NUM_LCD_CS 14
#define CUSTOM_PIN_NUM_LCD_DC 15
#define CUSTOM_PIN_NUM_LCD_RST 21
#define CUSTOM_PIN_NUM_BK_LIGHT 22
// The pixel number in horizontal and vertical
#define LCD_H_RES 172
#define LCD_V_RES 320
// Bit number used to represent command and parameter
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define Offset_X 34
#define Offset_Y 0

#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE
#define LEDC_HS_CH0_GPIO CUSTOM_PIN_NUM_BK_LIGHT
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0
#define LEDC_TEST_DUTY (4000)
#define LEDC_ResolutionRatio LEDC_TIMER_13_BIT
#define LEDC_MAX_Duty ((1 << LEDC_ResolutionRatio) - 1)

extern esp_lcd_panel_handle_t panel_handle;

void backlight_init(void);  // Initialize the LCD backlight, which has been called in the lcd_init function, ignore it
void backlight_brightness(uint8_t Light);  // Call this function to adjust the brightness of the backlight. The value of
                                           // the parameter Light ranges from 0 to 100

void lcd_init(void);  // Call this function to initialize the screen (must be called in the main function) !!!!!