#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_mac.h"

#define GATTC_TAG "GATTC_TAG"
#define SCAN_DURATION 5
#define MAX_DISCOVERED_DEVICES 100

typedef struct {
    uint8_t address[6];
    bool is_valid;
} discovered_device_t;

extern uint16_t BLE_NUM;
extern bool ble_scan_finish;
extern bool Scan_finish;

void ble_init(void *arg);
void ble_scan(void);
