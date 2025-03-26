#include "webserver.h"
#include "wifi.h"
#include "helper/json.h"
#include "time_sync.h"

esp_err_t status_handler(httpd_req_t *req);
esp_err_t wifi_scan_handler(httpd_req_t *req);
esp_err_t wifi_connect_handler(httpd_req_t *req);
esp_err_t ntp_set_handler(httpd_req_t *req);
