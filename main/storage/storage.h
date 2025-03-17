#pragma once

#include "nvs/nvs.h"
#include "sd_card/sd_card.h"

bool storage_init(void);
bool storage_create_directories(const char *path);
bool storage_has_extension(const char *path);