#include <time_sync.h>

static const char *TAG = "TIME_SYNC";
static int NTP_SYNC_TIMEOUT = 30;

/// @brief init system datetime with compiled time
/// @param void
/// @return void
void time_init_from_compile(void) {
    struct tm tm = {0};

    char month[4];
    int day, year, hour, minute, second;
    sscanf(__DATE__, "%3s %d %d", month, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

    int month_num = 0;

    if (strcmp(month, "Jan") == 0)
        month_num = 0;
    else if (strcmp(month, "Feb") == 0)
        month_num = 1;
    else if (strcmp(month, "Mar") == 0)
        month_num = 2;
    else if (strcmp(month, "Apr") == 0)
        month_num = 3;
    else if (strcmp(month, "May") == 0)
        month_num = 4;
    else if (strcmp(month, "Jun") == 0)
        month_num = 5;
    else if (strcmp(month, "Jul") == 0)
        month_num = 6;
    else if (strcmp(month, "Aug") == 0)
        month_num = 7;
    else if (strcmp(month, "Sep") == 0)
        month_num = 8;
    else if (strcmp(month, "Oct") == 0)
        month_num = 9;
    else if (strcmp(month, "Nov") == 0)
        month_num = 10;
    else if (strcmp(month, "Dec") == 0)
        month_num = 11;

    tm.tm_year = year - 1900;
    tm.tm_mon = month_num;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = -1;

    time_t t = mktime(&tm);
    if (t == -1) {
        ESP_LOGE(TAG, "Error on date conversion from firmware build date");
        return;
    }

    struct timeval tv = {.tv_sec = t, .tv_usec = 0};

    if (settimeofday(&tv, NULL) != 0) {
        ESP_LOGE(TAG, "Error on dateime update");
    } else {
        char datetime_str[26];
        strncpy(datetime_str, asctime(&tm), sizeof(datetime_str) - 1);
        datetime_str[strcspn(datetime_str, "\n")] = '\0';
        ESP_LOGI(TAG, "Initialized datetime : %s", datetime_str);
    }
}

/// @brief sync time with ntp server
/// @param char ntp_domain the ntp domain
/// @return bool true if change is ok, false otherwise
bool time_sync_with_ntp(const char *ntp_domain) {
    if (ntp_domain == NULL) {
        ESP_LOGE(TAG, "NTP domain cannot be NULL.");
        return false;
    }

    sntp_stop();

    ESP_LOGI(TAG, "Starting time sync with NTP server : %s", ntp_domain);

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, ntp_domain);
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;

    while (retry < NTP_SYNC_TIMEOUT) {
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_year >= (2020 - 1900)) {
            char datetime_str[26];
            strncpy(datetime_str, asctime(&timeinfo), sizeof(datetime_str) - 1);
            datetime_str[strcspn(datetime_str, "\n")] = '\0';
            ESP_LOGI(TAG, "Time synchronized successfully : %s", datetime_str);
            return true;
        }

        ESP_LOGW(TAG, "Waiting for NTP sync... (%d/%d)", retry + 1, NTP_SYNC_TIMEOUT);
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry++;
    }

    ESP_LOGE(TAG, "Failed to sync time with NTP server: %s", ntp_domain);
    return false;
}