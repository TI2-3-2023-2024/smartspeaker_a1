/* 
* Bestandsnaam: ntp.c
* Auteur: Luc van der Sar
* Datum: 12 maart 2024 
* Beschrijving: Dit bestand bevat de logica voor het ophalen van de actuele tijd.  
*/

#include "ntp.h"
#include "esp_log.h"

static const char *TAG = "dntp";

/* 
* Functie: ntp_set_timezone 
* Beschrijving: zet de juiste tijdzone in het systeem van de Time Library. 
* Parameters: een string in de vorm van de Time Library zoals : CET-1CEST,M3.5.0,M10.5.0/3
* Retourneert: Geen 
*/
void ntp_set_timezone(const char *timezone) {
    setenv("TZ", timezone, 1);
    tzset();
}

/* 
* Functie: ntp_initialize
* Beschrijving: Initializeert de tijd in het jaar 2024 
* Parameters: Geen
* Retourneert: Geen 
*/
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

/* 
* Functie: ntp_get_time
* Beschrijving: haalt de tijd op van de tijd die je in initialize in het geheugen hebt gezet  
* Parameters: 
* Retourneert: Geen 
*/
void ntp_get_time(char *time_str, size_t max_len) {
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_str, max_len, "%c", &timeinfo);
}
/* 
* Functie: ntp_test
* Beschrijving: een test methode om de actuele tijd te laten zien per 2 seconden   
* Parameters: Geen
* Retourneert: Geen 
*/
void ntp_test(void *pvParameters){
    for (;;)
    {
        char time_str[64]; // Buffer to store the time string
        ntp_get_time(time_str, sizeof(time_str));
        ESP_LOGI(TAG, "%s", time_str);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }   
}