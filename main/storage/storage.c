#include "storage.h"
#include <errno.h>

/// @brief Initialize the storage
/// @param void
/// @return bool : true if the storage is initialized, false otherwise
bool storage_init(void) {
    ESP_LOGI("STORAGE", "Initializing storage...");

    nvs_init();
    check_nvs_space();
    sd_init();
    sd_check_space();

    return true;
}

/// @brief Create directories
/// @param path The path to create directories
/// @return bool : true if the directories are created, false otherwise
bool storage_create_directories(const char *path) {
    char temp[256];
    char *p = NULL;
    size_t len;

    snprintf(temp, sizeof(temp), "%s", path);
    len = strlen(temp);
    if (temp[len - 1] == '/') {
        temp[len - 1] = 0;
    }

    if (storage_has_extension(temp)) {
        char *last_slash = strrchr(temp, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
        }
    }

    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST) {
                return false;
            }
            *p = '/';
        }
    }

    if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST) {
        return false;
    }

    return true;
}

/// @brief Return if a path has an extension
/// @param path The path to check
/// @return bool : true if the path has an extension, false otherwise
bool storage_has_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) return false;
    return true;
}