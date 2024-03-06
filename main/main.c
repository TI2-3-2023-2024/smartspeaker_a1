/*
Bestandsnaam: main.c
Auteur: Niels, Ruben en Thijme
Datum: 28-2-2024
Beschrijving: code om lcd menu aan te sturen voor sprint demo 1
*/

#include "menu.h"
#include "i2c_display.h"
#include "sd_card_player.h"

void init_i2c_display()
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG2, "I2C initialized successfully");

    // test data
    ESP_ERROR_CHECK(write_coordinates(7, 5));
    ESP_ERROR_CHECK(write_coordinates(0, 5));
    ESP_ERROR_CHECK(write_coordinates(31, 5));

    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG2, "I2C unitialized successfully");

    ESP_ERROR_CHECK(i2cdev_init());
}

void app_main()
{
    xTaskCreate(init_sd_card_player, "sd_card_player", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);

    init_i2c_display();

    // init lcd and start main menu
    // xTaskCreatePinnedToCore(init_lcd, "main_menu", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL, 1);
}
