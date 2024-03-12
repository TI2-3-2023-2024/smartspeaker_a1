#include "menu.h"



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

    vTaskDelete(NULL);
}