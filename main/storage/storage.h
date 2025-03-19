#pragma once

#include "nvs/nvs.h"
#include "sd_card/sd_card.h"
#include "spiffs_storage/spiffs_storage.h"

bool storage_init(void);
bool storage_create_directories(const char *path);
bool storage_has_extension(const char *path);
void storage_list_tree(const char *path, int indent);