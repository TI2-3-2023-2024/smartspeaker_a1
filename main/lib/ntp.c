#include "ntp.h"
#include "esp_log.h"

static const char *TAG = "dntp";

void ntp_set_timezone(const char *timezone) {
    setenv("TZ", timezone, 1);
    tzset();
}

void ntp_initialize(void) {
    ntp_set_timezone("CET-1CEST,M3.5.0,M10.5.0/3");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // Wait until time is obtained
    time_t now = 0;
    struct tm timeinfo = { 0 };
    while (timeinfo.tm_year < (2024 - 1900)) {
        ESP_LOGI(TAG, "Waiting for time synchronization...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void ntp_get_time(char *time_str, size_t max_len) {
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_str, max_len, "%c", &timeinfo);
}
