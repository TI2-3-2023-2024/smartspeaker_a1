#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include "esp_wifi.h"
#include "periph_wifi.h"

#define WIFI_SSID "iPhone van luc (2)"
#define WIFI_PASSWORD "password"


void wifi_setup_start();
extern esp_periph_set_handle_t wifiPeriphSet;


#endif /* WIFI_SETUP_H */
