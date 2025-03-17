#include "wireless.h"
#include "wifi/wifi.h"
#include "ble/ble.h"

bool Scan_finish = false;

bool wireless_Init(void) {
    xTaskCreatePinnedToCore(wifi_init, "wifi task", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(ble_init, "ble task", 4096, NULL, 2, NULL, 0);

    return true;
}
