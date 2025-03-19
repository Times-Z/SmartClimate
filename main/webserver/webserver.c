#include "webserver.h"

static const char *TAG = "HTTP_SERVER";
extern const char *SPIFFS_MOUNTPOINT;
static char CSS_PATH[128];

static esp_err_t log_request(httpd_req_t *req, esp_err_t status) {
    ESP_LOGI(TAG, "Request URI : %s | Status : %d", req->uri, status);
    return status;
}

static esp_err_t milligram_handler(httpd_req_t *req) {
    char resp[1024];
    FILE *file = fopen(CSS_PATH, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open %s", CSS_PATH);
        return log_request(req, ESP_FAIL);
    }
    size_t read_bytes = fread(resp, 1, sizeof(resp) - 1, file);
    fclose(file);
    if (read_bytes == 0) {
        ESP_LOGE(TAG, "Failed to read %s", CSS_PATH);
        return log_request(req, ESP_FAIL);
    }
    resp[read_bytes] = '\0';
    httpd_resp_set_type(req, "text/css");
    esp_err_t status = httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return log_request(req, status);
}

static esp_err_t setup(httpd_req_t *req) {
    char resp[1024];
    FILE *file = fopen("/spiffs/index.html", "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open /spiffs/index.html");
        return log_request(req, ESP_FAIL);
    }
    size_t read_bytes = fread(resp, 1, sizeof(resp) - 1, file);
    fclose(file);
    if (read_bytes == 0) {
        ESP_LOGE(TAG, "Failed to read /spiffs/index.html");
        return log_request(req, ESP_FAIL);
    }
    resp[read_bytes] = '\0';
    esp_err_t status = httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return log_request(req, status);
}

static esp_err_t connectivity_check(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Location", "http://smartclimate.local/setup");
    httpd_resp_set_status(req, "301 Moved Permanently");
    return log_request(req, httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN));
}

static esp_err_t default_handler(httpd_req_t *req, httpd_err_code_t err) {
    ESP_LOGW(TAG, "Undefined URI requested : %s | Error code : %d", req->uri, err);
    const char resp[] = "404 Not Found";
    esp_err_t status = httpd_resp_set_status(req, "404 Not Found");
    if (status == ESP_OK) {
        status = httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    }
    return log_request(req, status);
}

httpd_uri_t setup_uri = {.uri = "/setup", .method = HTTP_GET, .handler = setup, .user_ctx = NULL};
static httpd_uri_t connectivity_uris[] = {
    {.uri = "/generate_204", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
    {.uri = "/library/test/success.html", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
    {.uri = "/hotspot-detect.html", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
    {.uri = "/", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
};
httpd_uri_t css_uri = {
    .uri = "/css/milligram.min.css", .method = HTTP_GET, .handler = milligram_handler, .user_ctx = NULL};

httpd_handle_t webserver_start(void) {
    snprintf(CSS_PATH, sizeof(CSS_PATH), "%s/css/milligram.min.css", SPIFFS_MOUNTPOINT);
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(mdns_init());

    mdns_hostname_set(APP_NAME);
    mdns_instance_name_set("esp device");

    ESP_LOGI(TAG, "Webserver started on port : %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &setup_uri);
        httpd_register_uri_handler(server, &css_uri);

        for (size_t i = 0; i < sizeof(connectivity_uris) / sizeof(connectivity_uris[0]); i++) {
            httpd_register_uri_handler(server, &connectivity_uris[i]);
        }

        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, default_handler);

        mdns_service_add("HTTP webserver", "_http", "_tcp", 80, NULL, 0);

        return server;
    }
    ESP_LOGE(TAG, "Error on webserver boot");
    return NULL;
}
