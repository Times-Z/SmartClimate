#pragma once

#include "esp_log.h"
#include "nvs_flash.h"

bool nvs_init(void);
void check_nvs_space();
