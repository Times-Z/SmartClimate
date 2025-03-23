#pragma once
#include <lvgl.h>
#include <lv_qrcode.h>
#include <esp_log.h>
#include <stdio.h>
#include <string.h>

static const int UI_MAX_SIZE = 172;

// Global
lv_obj_t *ui_get_main_container(void);
void ui_display_app_name_and_version(void);

// Wireless
void ui_show_wifi_ap_qr(const char *ssid, const char *password);
void ui_display_ip(const char *ip_address);
void ui_show_idle_cat_animation(void);
