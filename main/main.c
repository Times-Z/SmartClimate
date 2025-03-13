#include "RGB.h"
#include "SD_SPI.h"
#include "ST7789.h"
#include "Wireless.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "MAIN"

void initialize_components(void) {
    ESP_LOGI(TAG, "Initializing system components...");

    if (Wireless_Init() != true) {
        ESP_LOGE(TAG, "Wireless initialization failed. Restarting...");
        esp_restart();
    }

    Flash_Searching();

    if (SD_Init() != true) {
        ESP_LOGE(TAG, "SD Card initialization failed. Running without SD support.");
    }

    RGB_Init();
    RGB_Example();

    LCD_Init();
    BK_Light(20);
    LVGL_Init();
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting SmartClimate...");
    initialize_components();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
