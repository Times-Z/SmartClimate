#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"

#include <stdio.h>
#include <string.h>
#include "esp_system.h"

extern uint16_t WIFI_NUM;
extern bool WiFi_Scan_Finish;
extern bool Scan_finish;

void wifi_init(void* arg);
void wifi_scan(void);
const char* authmode_to_str(wifi_auth_mode_t authmode);
void wifi_create_ap(const char* ssid, const char* password);
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
bool wifi_is_known_network_available(void);
