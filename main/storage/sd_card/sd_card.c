#include "sd_card.h"

#define EXAMPLE_MAX_CHAR_SIZE 64
const char *SD_MOUNT_POINT = "/sdcard";

static const char *SD_TAG = "SD";

uint32_t Flash_Size = 0;
uint32_t SDCard_Size = 0;

esp_err_t s_example_write_file(const char *path, char *data) {
    ESP_LOGI(SD_TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(SD_TAG, "File written");

    return ESP_OK;
}

esp_err_t s_example_read_file(const char *path) {
    ESP_LOGI(SD_TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(SD_TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

/// @brief Initialize the SD card
/// @param void
/// @return bool : true if the SD card is initialized, false otherwise
bool sd_init(void) {
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true, .max_files = 5, .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;

    ESP_LOGI(SD_TAG, "Initializing SD card");

    ESP_LOGI(SD_TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);

    if (ret != ESP_OK) {
        ESP_LOGE(SD_TAG, "Failed to initialize SPI bus.");
        return false;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(SD_TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SD_TAG,
                     "Failed to mount filesystem. If you want the card to be formatted, set the "
                     "CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(SD_TAG,
                     "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return false;
    }
    ESP_LOGI(SD_TAG, "Filesystem mounted");

    SDCard_Size = ((uint64_t)card->csd.capacity) * card->csd.sector_size / (1024 * 1024);
    bus_cfg.max_transfer_sz = card->max_freq_khz / 8;

    ESP_LOGI(SD_TAG, "SD Card Size: %lu MB", SDCard_Size);
    ESP_LOGI(SD_TAG, "Max transfer size: %lu Mhz", card->max_freq_khz);

    return true;
}

/// @brief Check the space available in the SD card
/// @param void
/// @return void
void sd_check_space(void) {
    FATFS *fs;
    DWORD free_clust;
    int free_sect = 0, tot_sect = 0;

    if (f_getfree(SD_MOUNT_POINT, &free_clust, &fs) == FR_OK) {
        tot_sect = (fs->n_fatent - 2) * fs->csize;
        free_sect = free_clust * fs->csize;

        float tot_mb = (tot_sect * 512.0) / (1024 * 1024);
        float free_mb = (free_sect * 512.0) / (1024 * 1024);
        float tot_gb = tot_mb / 1024;
        float free_gb = free_mb / 1024;

        ESP_LOGI(SD_TAG, "FATFS Info :");
        ESP_LOGI(SD_TAG, "Total size: %.2f MB (%.2f GB)", tot_mb, tot_gb);
        ESP_LOGI(SD_TAG, "Free size: %.2f MB (%.2f GB)", free_mb, free_gb);

        if (free_sect < (tot_sect * 0.2)) {
            ESP_LOGW(SD_TAG, "SD Card memory is almost full, used sectors = (%u), total sectors = (%u)",
                     tot_sect - free_sect, tot_sect);
        }

    } else {
        ESP_LOGE(SD_TAG, "Failed to get free space information");
    }
}
