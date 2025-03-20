#include "webserver.h"

static const char *TAG = "HTTP_SERVER";
extern const char *SPIFFS_MOUNTPOINT;
static char CSS_PATH[128];
static char JS_PATH[128];

/// @brief Serve a file from the SPIFFS filesystem
/// @param req httpd_req_t
/// @param file_path const char*
/// @param content_type const char*
/// @return esp_err_t
static esp_err_t serve_file(httpd_req_t *req, const char *file_path, const char *content_type) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", file_path);
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open file");
    }

    httpd_resp_set_type(req, content_type);
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        esp_err_t send_status = httpd_resp_send_chunk(req, buffer, bytes_read);
        if (send_status != ESP_OK) {
            fclose(file);
            ESP_LOGE(TAG, "Error sending chunk from file: %s", file_path);
            return send_status;
        }
    }
    fclose(file);
    return httpd_resp_send_chunk(req, NULL, 0);
}

/// @brief Log the request URI and status
/// @param req httpd_req_t
/// @param status esp_err_t
/// @return esp_err_t
static esp_err_t log_request(httpd_req_t *req, esp_err_t status) {
    ESP_LOGI(TAG, "Request URI : %s | Status : %d", req->uri, status);
    return status;
}

/// @brief setup page handler
/// @param req httpd_req_t
/// @return esp_err_t
static esp_err_t setup_handler(httpd_req_t *req) {
    esp_err_t status = serve_file(req, "/spiffs/index.html", "text/html");
    return log_request(req, status);
}

/// @brief Handler for milligram CSS
/// @param req httpd_req_t
/// @return esp_err_t
static esp_err_t milligram_handler(httpd_req_t *req) {
    esp_err_t status = serve_file(req, CSS_PATH, "text/css");
    return log_request(req, status);
}

/// @brief Handler for milligram CSS
/// @param req httpd_req_t
/// @return esp_err_t
static esp_err_t umbrella_handler(httpd_req_t *req) {
    esp_err_t status = serve_file(req, JS_PATH, "text/javascript");
    return log_request(req, status);
}

/// @brief Connectivity check handler (captive portal)
/// @param req httpd_req_t
/// @return esp_err_t
static esp_err_t connectivity_check(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Location", "http://smartclimate/setup");
    httpd_resp_set_status(req, "301 Moved Permanently");
    esp_err_t status = httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
    return log_request(req, status);
}

/// @brief Default request handler for undefined URIs
/// @param req httpd_req_t
/// @param err httpd_err_code_t
/// @return esp_err_t
static esp_err_t default_handler(httpd_req_t *req, httpd_err_code_t err) {
    ESP_LOGW(TAG, "Undefined URI requested: %s | Error code: %d", req->uri, err);
    const char resp[] = "404 Not Found";
    esp_err_t status = httpd_resp_set_status(req, "404 Not Found");
    if (status == ESP_OK) {
        status = httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    }
    return log_request(req, status);
}

/// @brief Start the webserver
/// @param void
/// @return httpd_handle_t
httpd_handle_t webserver_start(void) {
    snprintf(CSS_PATH, sizeof(CSS_PATH), "%s/css/milligram.min.css", SPIFFS_MOUNTPOINT);
    snprintf(JS_PATH, sizeof(JS_PATH), "%s/js/umbrella.min.js", SPIFFS_MOUNTPOINT);

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t setup_uri = {.uri = "/setup", .method = HTTP_GET, .handler = setup_handler, .user_ctx = NULL};

    static httpd_uri_t connectivity_uris[] = {
        {.uri = "/generate_204", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
        {.uri = "/library/test/success.html", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
        {.uri = "/hotspot-detect.html", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
        {.uri = "/", .method = HTTP_GET, .handler = connectivity_check, .user_ctx = NULL},
    };

    httpd_uri_t css_uri = {
        .uri = "/css/milligram.min.css", .method = HTTP_GET, .handler = milligram_handler, .user_ctx = NULL};
    httpd_uri_t js_uri = {
        .uri = "/js/umbrella.min.js", .method = HTTP_GET, .handler = umbrella_handler, .user_ctx = NULL};

    ESP_LOGI(TAG, "Webserver starting on port: %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &setup_uri);
        httpd_register_uri_handler(server, &css_uri);
        httpd_register_uri_handler(server, &js_uri);

        for (size_t i = 0; i < sizeof(connectivity_uris) / sizeof(connectivity_uris[0]); i++) {
            httpd_register_uri_handler(server, &connectivity_uris[i]);
        }

        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, default_handler);
        return server;
    }
    ESP_LOGE(TAG, "Error on webserver boot");
    return NULL;
}
