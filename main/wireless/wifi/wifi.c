#include "wifi.h"
#include "../../ui/global.h"
#include "../captive_portal/captive_portal.h"

#define MAX_APs 20
#define WIFI_RETRY_MAX 2

extern const char *APP_NAME;
static const char *AP_PASSWORD = "$tr0ngWifi";

static const char *TAG = "WIFI";
static int s_retry_num = 0;
static wifi_ap_record_t ap_records[MAX_APs];
static esp_netif_t *ap_netif = NULL;
static esp_netif_t *sta_netif = NULL;
bool should_save_credentials = false;

static char pending_ssid[33] = {0};
static char pending_password[65] = {0};

static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_RETRY_MAX) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying Wi-Fi connection... (attempt %d)", s_retry_num);
        } else {
            ESP_LOGW(TAG, "Failed after %d attempts, clearing creds and starting AP mode", WIFI_RETRY_MAX);
            nvs_clear_wifi_credentials();
            memset(pending_ssid, 0, sizeof(pending_ssid));
            memset(pending_password, 0, sizeof(pending_password));
            should_save_credentials = false;
            wifi_start_ap();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));

        ESP_LOGI(TAG, "Connected successfully with IP : %s", ip_str);

        ui_display_app_name_and_version();
        ui_display_ip(ip_str);
        ui_show_idle_cat_animation();

        if (should_save_credentials && strlen(pending_ssid) > 0 && strlen(pending_password) > 0) {
            nvs_save_wifi_credentials(pending_ssid, pending_password);
            ESP_LOGI(TAG, "Wi-Fi credentials saved to NVS (API call)");
        } else {
            ESP_LOGI(TAG, "Wi-Fi credentials NOT saved to NVS");
        }

        should_save_credentials = false;
        memset(pending_ssid, 0, sizeof(pending_ssid));
        memset(pending_password, 0, sizeof(pending_password));
    }
}

void wifi_start_ap(void) {
    if (ap_netif != NULL) {
        esp_netif_dhcps_stop(ap_netif);
        esp_netif_destroy_default_wifi(ap_netif);
        ap_netif = NULL;
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    captive_portal_stop_dns_server();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = false;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ap_netif = esp_netif_create_default_wifi_ap();
    if (ap_netif == NULL) {
        ESP_LOGE(TAG, "Error creating AP netif");
        return;
    }

    wifi_config_t ap_config = {0};
    strncpy((char *)ap_config.ap.ssid, APP_NAME, sizeof(ap_config.ap.ssid));
    strncpy((char *)ap_config.ap.password, AP_PASSWORD, sizeof(ap_config.ap.password));
    ap_config.ap.ssid_len = strlen(APP_NAME);
    ap_config.ap.channel = 1;
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config.ap.max_connection = 4;
    ap_config.ap.beacon_interval = 100;

    if (strlen(AP_PASSWORD) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_dhcps_stop(ap_netif);
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 10, 0, 1, 1);
    IP4_ADDR(&ip_info.gw, 10, 0, 1, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));

    esp_netif_dns_info_t dns_info;
    dns_info.ip.u_addr.ip4.addr = ip_info.ip.addr;
    dns_info.ip.type = ESP_IPADDR_TYPE_V4;
    ESP_ERROR_CHECK(esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns_info));

    esp_netif_dhcps_start(ap_netif);

    captive_portal_start_dns_server();

    ESP_LOGI(TAG, "Static IP configured: %s", ip_to_str(&ip_info.ip));
    ui_show_wifi_ap_qr((char *)ap_config.ap.ssid, (char *)ap_config.ap.password);
}

bool wifi_start_sta(const char *ssid, const char *password) {
    if (!ssid || !password || strlen(ssid) == 0 || strlen(password) == 0) {
        ESP_LOGE(TAG, "Invalid ssid/password provided.");
        return false;
    }

    esp_wifi_stop();  // Safe car déjà init dans wifi_init()
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();

    ESP_LOGI(TAG, "Connecting to Wi-Fi SSID: %s", ssid);

    strncpy(pending_ssid, ssid, sizeof(pending_ssid));
    strncpy(pending_password, password, sizeof(pending_password));

    return true;
}

/// @brief init wifi, wrapper for ap/station mode
/// @param void
/// @return void
void wifi_init(void) {
    s_retry_num = 0;

    ESP_ERROR_CHECK(esp_netif_init());

    sta_netif = esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = false;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, &instance_got_ip));

    char ssid_local[33] = {0};
    char password_local[65] = {0};
    if (nvs_load_wifi_credentials(ssid_local, sizeof(ssid_local), password_local, sizeof(password_local))) {
        wifi_start_sta(ssid_local, password_local);
    } else {
        ESP_LOGI(TAG, "No Wi-Fi credentials in NVS, starting AP mode");
        wifi_start_ap();
    }
}

/// @brief perform Wi-Fi scan and directly retrieve the results.
/// @param results Pointer to store the AP scan results.
/// @param ap_count Pointer to store the number of APs found.
/// @return esp_err_t ESP_OK on success or appropriate error code.
esp_err_t wifi_scan_networks(wifi_ap_record_t **results, uint16_t *ap_count) {
    wifi_scan_config_t scan_conf = {.ssid = NULL, .bssid = NULL, .channel = 0, .show_hidden = true};

    esp_err_t err;

    ESP_ERROR_CHECK(esp_wifi_start());

    err = esp_wifi_scan_start(&scan_conf, true);
    if (err != ESP_OK) return err;

    err = esp_wifi_scan_get_ap_num(ap_count);
    if (err != ESP_OK) return err;

    if (*ap_count > MAX_APs) *ap_count = MAX_APs;

    err = esp_wifi_scan_get_ap_records(ap_count, ap_records);
    if (err != ESP_OK) return err;

    ESP_LOGI(TAG, "Number of APs found: %d", *ap_count);
    *results = ap_records;

    return ESP_OK;
}
