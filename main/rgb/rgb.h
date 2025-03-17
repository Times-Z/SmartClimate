#pragma once

#include "driver/gpio.h"
#include "led_strip.h"

#define BLINK_GPIO 8

void rgb_init(void);
void set_rgb(uint8_t red_val, uint8_t green_val, uint8_t blue_val);
void rgb_example(void);