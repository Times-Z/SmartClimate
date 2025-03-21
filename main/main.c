#include "time_sync.h"
#include "rgb.h"
#include "st7789.h"
#include "storage.h"
#include "wireless.h"
#include "log.h"
#include "webserver.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_app_desc.h"

static const char *MAIN_TAG = "MAIN";
const char *APP_NAME;
const char *APP_VERSION;

void initialize_components(void) {
    ESP_LOGI(MAIN_TAG, "Initializing system components...");

    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(MAIN_TAG, "Error on default event loop creation : %d", err);
        esp_restart();
    }

    time_init_from_compile();

    if (storage_init() != true) {
        ESP_LOGE(MAIN_TAG, "storage init failed. Stopping...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_deep_sleep_start();
    }

    if (wireless_Init() != true) {
        ESP_LOGE(MAIN_TAG, "wireless initialization failed. Restarting...");
        esp_restart();
    }

    webserver_start();

    rgb_init();
    rgb_example();

    lcd_init();
    backlight_brightness(20);
    lvgl_init();
}

void app_main(void) {
    APP_NAME = esp_app_get_description()->project_name;
    APP_VERSION = esp_app_get_description()->version;

    ESP_LOGI(MAIN_TAG, "Starting %s version %s...", APP_NAME, APP_VERSION);
    initialize_components();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
