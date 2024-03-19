/*
Bestandsnaam: main.c
Auteur: Niels, Ruben en Thijme
Datum: 28-2-2024
Beschrijving: code om lcd menu aan te sturen voor sprint demo 1
*/

#include <inttypes.h>

#include "menu.h"
#include "i2c_display.h"
#include "lib\buttonreader.h"
#include "sharedvariable.h"

#include "lib/internet_radio.h"
#include "lib/ntp.h"
#include "lib/wifi_setup.h"
#include "lib/sd_card_player.h"

SemaphoreHandle_t xMutex;

void i2c_display_test()
{
    i2c_master_init();

    srand(time(NULL));

    for (int i = 0; i < 16; i++)
    {
        int random = rand() % 8;
        ESP_ERROR_CHECK(write_coordinates(i, random));
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void app_main()
{
    // // Start Wi-Fi setup
    wifi_setup_start();

    char time_str[64];
    ESP_LOGI("main", "Initializing NTP...");
    ntp_initialize();
    ESP_LOGI("main", "NTP initialized.");

    // //test task voor de actuele tijd.
    // xTaskCreate(ntp_test, "main_menu", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);

    xMutex = xSemaphoreCreateMutex();

    ESP_ERROR_CHECK(i2cdev_init());

    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    // //test data
    // ESP_ERROR_CHECK(write_coordinates(7, 5));
    // ESP_ERROR_CHECK(write_coordinates(0, 5));
    // ESP_ERROR_CHECK(write_coordinates(31, 5));

    // ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    //  ESP_LOGI(TAG, "I2C unitialized successfully");

    // init lcd and start main menu

    // check if mutex correctly made and creates "start_reader" task
    if (xMutex != NULL)
    {

        xTaskCreate(start_reader, "start reader", configMINIMAL_STACK_SIZE * 6, NULL, 5, NULL);
        // xTaskCreate(init_sd_card_player, "init_sd_card_player", configMINIMAL_STACK_SIZE * 6, NULL, 5, NULL);
        i2c_display_test();
    }
}
