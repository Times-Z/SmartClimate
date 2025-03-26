#include "global.h"

static const char *TAG = "UI:WIRELESS";

void ui_show_wifi_ap_qr(const char *ssid, const char *password) {
    ESP_LOGI(TAG, "Generating QR code...");

    char qr_text[128];
    snprintf(qr_text, sizeof(qr_text), "WIFI:T:WPA;S:%s;P:%s;;", ssid, password);

    lv_obj_t *container = ui_get_main_container();

    // QR Code
    lv_obj_t *qr = lv_qrcode_create(container);

    lv_qrcode_set_size(qr, UI_MAX_SIZE * 0.9);
    lv_qrcode_set_dark_color(qr, lv_color_black());
    lv_qrcode_set_light_color(qr, lv_color_white());
    lv_obj_set_style_bg_color(qr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(qr, LV_OPA_COVER, 0);
    lv_qrcode_update(qr, qr_text, strlen(qr_text));
    lv_obj_align(qr, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t *txt_label = lv_label_create(container);
    lv_label_set_text_fmt(txt_label, "Connect to configure");
    lv_obj_clear_flag(txt_label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_align(txt_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(txt_label, qr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Label texte SSID + PWD
    lv_obj_t *info_label = lv_label_create(container);
    lv_label_set_text_fmt(info_label, "SSID: %s\nPWD: %s", ssid, password);
    lv_obj_clear_flag(info_label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(info_label, txt_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void ui_show_ip(const char *ip_address) {
    ESP_LOGI(TAG, "Displaying ip...");

    if (ip_address == NULL) {
        ESP_LOGW(TAG, "IP address is NULL, cannot display.");
        return;
    }

    static lv_obj_t *ip_label = NULL;

    if (ip_label == NULL) {
        lv_obj_t *container = ui_get_main_container();
        ip_label = lv_label_create(container);
        lv_obj_set_style_text_color(ip_label, lv_color_black(), 0);
        lv_obj_set_width(ip_label, lv_disp_get_hor_res(NULL));
        lv_label_set_long_mode(ip_label, LV_LABEL_LONG_WRAP);
    }

    lv_label_set_text_fmt(ip_label, "IP : %s", ip_address);
}