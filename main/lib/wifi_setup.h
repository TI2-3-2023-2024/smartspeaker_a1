#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include "esp_wifi.h"
#include "periph_wifi.h"

#define WIFI_SSID "NielsMagNiet"
#define WIFI_PASSWORD "Banaan123"


void wifi_setup_start();
extern esp_periph_set_handle_t wifiPeriphSet;


#endif /* WIFI_SETUP_H */
