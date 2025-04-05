#pragma once
#include <lvgl.h>
#include <lv_qrcode.h>
#include <esp_log.h>
#include <stdio.h>
#include <string.h>

static const int UI_MAX_SIZE = 172;

// Global
void ui_clear_screen(void);
lv_obj_t *ui_get_main_container(void);
void ui_clear_main_container(void);
void ui_show_app_name_and_version(void);
void ui_show_message(char *message);
void ui_show_loader_spinner(void);
void ui_show_gif(const void *src, uint32_t zoom_ratio);

// Wireless
void ui_show_wifi_ap_qr(const char *ssid, const char *password);
void ui_show_ip(const char *ip_address);
void ui_show_idle_cat_animation(void);
