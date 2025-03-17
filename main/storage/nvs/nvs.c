#include "nvs.h"

/// @brief  Initialize the NVS flash memory (non volatile storage)
/// @param void
/// @return bool : true if the NVS flash memory is initialized, false otherwise
bool nvs_init(void) {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW("NVS", "NVS error detected, flash erasing...");
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            ESP_LOGE("NVS", "Error on erasing : %s", esp_err_to_name(ret));
            return false;
        }

        ret = nvs_flash_init();
    }

    if (ret != ESP_OK) {
        ESP_LOGE("NVS", "Error on init nvs : %s", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI("NVS", "NVS initialized");
    return true;
}

/// @brief  Check the space available in the NVS flash memory
/// @param void
/// @return void
void check_nvs_space() {
    nvs_stats_t nvs_stats;
    esp_err_t err = nvs_get_stats(NULL, &nvs_stats);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to get NVS stats (%s)", esp_err_to_name(err));
        return;
    }

    if (nvs_stats.used_entries > (nvs_stats.total_entries * 0.8)) {
        ESP_LOGW("NVS", "NVS memory is almost full, used entries = (%d), total entries = (%d)", nvs_stats.used_entries,
                 nvs_stats.total_entries);
    } else {
        ESP_LOGI("NVS", "NVS memory used entries = (%d), total entries = (%d)", nvs_stats.used_entries,
                 nvs_stats.total_entries);
    }
}
