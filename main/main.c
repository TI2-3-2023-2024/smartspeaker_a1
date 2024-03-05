/*
Bestandsnaam: main.c
Auteur: Niels, Ruben en Thijme
Datum: 28-2-2024
Beschrijving: code om lcd menu aan te sturen voor sprint demo 1
*/

#include <inttypes.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>
#include <hd44780.h>
#include <pcf8574.h>
#include <string.h>

#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "i2c-simple-example";

#define I2C_MASTER_SCL_IO 23        /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 18        /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define MPU9250_SENSOR_ADDR 0x04       /*!< Slave address of the MPU9250 sensor */
#define MPU9250_WHO_AM_I_REG_ADDR 0x75 /*!< Register addresses of the "who am I" register */

#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B /*!< Register addresses of the power managment register */
#define MPU9250_RESET_BIT 7

typedef struct
{
    int id;
    char *song_name;
} song;

void input_menu();
void main_menu();
void song_selection_menu(song songs[], size_t size);

static i2c_dev_t pcf8574;

static hd44780_t lcd;

static const uint8_t char_data[] = {
    0x04, 0x0e, 0x0e, 0x0e, 0x1f, 0x00, 0x04, 0x00,
    0x1f, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x1f, 0x00};

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

void main_menu()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "SELECT TYPE");
    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "SD | API | MIC | x");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        input_menu();
        return;
    }
}

void input_menu()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "INPUT");

    hd44780_gotoxy(&lcd, 1, 1);
    hd44780_puts(&lcd, "TALK INTO THE MIC");

    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK | x | x | x");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));

        song songs[3] = {
            {1, "brood"},
            {2, "plankje"},
            {3, "knakworst"}};

        size_t num_songs = sizeof(songs) / sizeof(songs[0]);

        song_selection_menu(songs, num_songs);
        return;
    }
}

void song_selection_menu(song songs[], size_t size)
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "SELECT SONG");

    char song_count[20];

    sprintf(song_count, "%d/%d", songs[0].id, size);

    hd44780_gotoxy(&lcd, 15, 0);
    hd44780_puts(&lcd, song_count);

    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, "SONG:");

    hd44780_gotoxy(&lcd, 6, 1);
    hd44780_puts(&lcd, songs[0].song_name);

    hd44780_gotoxy(&lcd, 0, 2);
    hd44780_puts(&lcd, "NEXT: ");

    hd44780_gotoxy(&lcd, 6, 2);
    hd44780_puts(&lcd, songs[1].song_name);

    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK | <- | -> | OK");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        main_menu();
        return;
    }
}

void init_lcd(void *pvParameters)
{
    hd44780_t lcd_display = {
        .write_cb = write_lcd_data, // use callback to send data to LCD by I2C GPIO expander
        .font = HD44780_FONT_5X8,
        .lines = 4,
        .pins = {
            .rs = 0,
            .e = 2,
            .d4 = 4,
            .d5 = 5,
            .d6 = 6,
            .d7 = 7,
            .bl = 3}};

    lcd = lcd_display;

    memset(&pcf8574, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, CONFIG_EXAMPLE_I2C_ADDR, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    ESP_ERROR_CHECK(hd44780_init(&lcd));

    hd44780_switch_backlight(&lcd, true);

    hd44780_upload_character(&lcd, 0, char_data);
    hd44780_upload_character(&lcd, 1, char_data + 8);

    // test array of songs
    // todo replace with songs from sd card or api

    main_menu();
}

static esp_err_t write_coordinates(uint8_t x, uint8_t y)
{
    int ret;
    uint8_t write_buf[2] = {x, y};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

    return ret;
}

/* 
* Functie: i2c_master_init
* Beschrijving: Initialiseert dit device als een i2c master
* Parameters: Geen 
* Retourneert: Status van installatie i2c drivers
*/
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void app_main()
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    //test data
    ESP_ERROR_CHECK(write_coordinates(7, 5));
    ESP_ERROR_CHECK(write_coordinates(0, 5));
    ESP_ERROR_CHECK(write_coordinates(31, 5));

    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG, "I2C unitialized successfully");

    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(init_lcd, "main_menu", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
}
