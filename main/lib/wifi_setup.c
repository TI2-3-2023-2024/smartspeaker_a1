/* 
* Bestandsnaam: wifi_setup.c 
* Auteur: Luc van der Sar
* Datum: 12 maart 2024 
* Beschrijving: Dit bestand bevat de functionaliteiten om een verbinding op te stellen met het Internet. 
* Config : SSID en wachtwoord staan in het Header bestand. 
*/

#include "wifi_setup.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "periph_wifi.h"
#include "nvs_flash.h"
#include "lib/connection_animation.h"

esp_periph_set_handle_t wifiPeriphSet;

static const char *TAG = "WIFI_SETUP";

/* 
* Functie: wifi_setup_start
* Beschrijving: Initialiseert en start de wifi verbinding 
* Parameters: Geen 
* Retourneert: Geen 
*/

void wifi_setup_start() {
    
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) 
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
    ESP_ERROR_CHECK(esp_netif_init());
#else
    tcpip_adapter_init();
#endif

    ESP_LOGI(TAG, "Start and wait for Wi-Fi network");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    wifiPeriphSet = esp_periph_set_init(&periph_cfg);
    periph_wifi_cfg_t wifi_cfg = {
        .wifi_config.sta.ssid = WIFI_SSID,
        .wifi_config.sta.password = WIFI_PASSWORD,
    };
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);
    
  
    start_connect_animation();
    esp_periph_start(wifiPeriphSet, wifi_handle);
    
    TaskHandle_t connect_animation_handle = NULL;
    xTaskCreate(establish_connect_animation_task, "establish_connect_animation_task", configMINIMAL_STACK_SIZE * 4, NULL, 1,&connect_animation_handle);
    
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);
    vTaskDelete(connect_animation_handle);
    finish_connect_animation();
   
}
