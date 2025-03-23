#pragma once

#include <lvgl.h>
#include <esp_lcd_panel_interface.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_heap_caps.h>
#include <string.h>

extern lv_display_t *disp;
void *lv_malloc_core(size_t size);
void lv_free_core(void *ptr);
void *lv_realloc_core(void *ptr, size_t new_size);
void lv_mem_init(void);

void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void lvgl_port_update_callback(lv_display_t *disp);
void lvgl_init(void);
