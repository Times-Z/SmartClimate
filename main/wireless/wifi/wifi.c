#include "wifi.h"

extern bool ble_scan_finish;

uint16_t WIFI_NUM = 0;
bool WiFi_Scan_Finish = 0;
extern char *APP_NAME;

static const char *WIFI_TAG = "WIFI";
static const char *KNOWN_SSID = "";
static const char *KNOWN_PASSWORD = "";
static const char *AP_PASSWORD = "";

const char *authmode_to_str(wifi_auth_mode_t authmode) {
    switch (authmode) {
        case WIFI_AUTH_OPEN:
            return "Open";
        case WIFI_AUTH_WEP:
            return "WEP";
        case WIFI_AUTH_WPA_PSK:
            return "WPA-PSK";
        case WIFI_AUTH_WPA2_PSK:
            return "WPA2-PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA/WPA2-PSK";
        case WIFI_AUTH_WPA3_PSK:
            return "WPA3-PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2/WPA3-PSK";
        case WIFI_AUTH_WAPI_PSK:
            return "WAPI-PSK";
        case WIFI_AUTH_OWE:
            return "OWE";
        case WIFI_AUTH_WPA3_ENT_192:
            return "WPA3-Enterprise-192";
        default:
            return "Unknown method";
    }
}

/// @brief Connect to a WiFi network or create an Access Point
/// @param arg
/// @return void
void wifi_init(void *arg) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    if (wifi_is_known_network_available()) {
        wifi_config_t sta_config = {
            .sta =
                {
                    .ssid = "",
                    .password = "",
                },
        };
        strncpy((char *)sta_config.sta.ssid, KNOWN_SSID, sizeof(sta_config.sta.ssid));
        strncpy((char *)sta_config.sta.password, KNOWN_PASSWORD, sizeof(sta_config.sta.password));

        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else {
        wifi_create_ap(APP_NAME, AP_PASSWORD);
    }

    vTaskDelete(NULL);
}

/// @brief Scan for WiFi networks
/// @param void
/// @return void
void wifi_scan(void) {
    uint16_t net_count = 0;
    wifi_ap_record_t *net_records = NULL;

    ESP_LOGI(WIFI_TAG, "Wifi scan ...");
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&net_count));

    if (net_count == 0) {
        ESP_LOGW(WIFI_TAG, "No access point found");
        return;
    }

    net_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * net_count);
    if (!net_records) {
        ESP_LOGE(WIFI_TAG, "Memory allocation error");
        return;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&net_count, net_records));

    printf("%d wifi networks discovered :\n", net_count);
    for (int i = 0; i < net_count; i++) {
        wifi_ap_record_t *ap = &net_records[i];
        printf("[%2d] SSID: %-32s | RSSI: %3d dBm | Authmode: %-15s\n", i, (char *)ap->ssid, ap->rssi,
               authmode_to_str(ap->authmode));
    }

    free(net_records);

    WiFi_Scan_Finish = 1;
    if (ble_scan_finish) Scan_finish = 1;
}

/// @brief Create a WiFi Access Point using the default ESP-NETIF AP interface.
/// @param ssid The SSID of the Access Point.
/// @param password The password of the Access Point.
void wifi_create_ap(const char *ssid, const char *password) {
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    if (ap_netif == NULL) {
        ESP_LOGE(WIFI_TAG, "Error on creating default AP interface");
        return;
    }

    wifi_config_t ap_config = {
        .ap =
            {
                .ssid = "",
                .password = "",
                .ssid_len = strlen(ssid),
                .channel = 1,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .ssid_hidden = 0,
                .max_connection = 1,
                .beacon_interval = 100,
            },
    };
    strncpy((char *)ap_config.ap.ssid, ssid, sizeof(ap_config.ap.ssid));
    strncpy((char *)ap_config.ap.password, password, sizeof(ap_config.ap.password));

    if (strlen(password) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_err_t err = esp_netif_dhcps_stop(ap_netif);
    if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_NOT_STOPPED) {
        ESP_LOGE(WIFI_TAG, "Error on dhcp stop : %s", esp_err_to_name(err));
        return;
    }

    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 10, 0, 1, 1);
    IP4_ADDR(&ip_info.gw, 10, 0, 1, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));

    esp_netif_dns_info_t dns_info;
    dns_info.ip.u_addr.ip4.addr = ip_info.ip.addr;
    dns_info.ip.type = ESP_IPADDR_TYPE_V4;
    ESP_ERROR_CHECK(esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif));

    captive_portal_start_dns_server();

    ESP_LOGI(WIFI_TAG, "Static IP reconfigured on %s", ip_to_str(&ip_info.ip));

    ESP_LOGI(WIFI_TAG, "WiFi Access Point '%s' created with auth method %s", ssid,
             authmode_to_str(ap_config.ap.authmode));
}

/// @brief Check if a known network is available
/// @param void
/// @return bool : True if the network is available, false otherwise
bool wifi_is_known_network_available(void) {
    uint16_t net_count = 0;
    wifi_ap_record_t *net_records = NULL;

    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&net_count));

    if (net_count == 0) {
        return false;
    }

    net_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * net_count);
    if (!net_records) {
        ESP_LOGE(WIFI_TAG, "Memory allocation error");
        return false;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&net_count, net_records));

    for (int i = 0; i < net_count; i++) {
        if (strcmp((char *)net_records[i].ssid, KNOWN_SSID) == 0) {
            free(net_records);
            return true;
        }
    }

    free(net_records);
    return false;
}

/// @brief WiFi event handler
/// @param arg The argument
/// @param event_base The event base
/// @param event_id The event ID
/// @param event_data The event data
/// @return void
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(WIFI_TAG, "Disconnected from WiFi, retrying...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "Current IP address : " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

char *ip_to_str(const esp_ip4_addr_t *ip) {
    static char ip_str[16];
    snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip4_addr1(ip), ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
    return ip_str;
}