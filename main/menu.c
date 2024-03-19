/*
Bestandsnaam: menu.c
Auteur: Niels en Ruben en Luc
Datum: 18-3-2024
Beschrijving: code voor het tonen en navigeren van het menu op het lcd scherm
*/

#include <time.h>
#include "menu.h"
#include "sharedvariable.h"
#include "lib/internet_radio.h"
#include "lib/sd_card_player.h"
#include "i2c_display.h"

static i2c_dev_t pcf8574;

static hd44780_t lcd;

// pages
menu_page current_page;

menu_page main_page;
menu_page input_page;
menu_page song_selection_page;
menu_page radio_selection_page;
menu_page song_play_page;
menu_page radio_play_page;
menu_page time_menu_page;

char **song_names;
int playlist_size;

const char *day_format = "%H:%M:%S";
const char *day_month_year_format = "%d %b %Y %H:%M";
const char *full_format = "%A, %d %m %Y  %H:%M:%S";

radio_station selected_station;
static radio_station stations[3];
static song *songs;
static int station_index = 0;
TaskHandle_t radio_task_handle;
TaskHandle_t sd_task_handle;

uint8_t brightness_leds = 240;

static const uint8_t char_data[] = {
    0x04, 0x0e, 0x0e, 0x0e, 0x1f, 0x00, 0x04, 0x00,
    0x1f, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x1f, 0x00};

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

TaskHandle_t time_update_task_handle = NULL;

/*
 * Function: initialiseer de setting voor het LCD scherm
 * Parameters: None
 * Returns: None
 */
void init_lcd()
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
}

/*
 * Function: initialiseer de inhoud van de radio pagina
 * Parameters: None
 * Returns: None
 */
void radio_page_init()
{
    stations[0] = (radio_station){1, "Sunrise", "http://185.66.249.48:8006/stream?type=.mp3"};
    stations[1] = (radio_station){2, "Jumbo FM", "https://playerservices.streamtheworld.com/api/livestream-redirect/JUMBORADIO.mp3"};
    stations[2] = (radio_station){3, "Slam FM", "https://stream.slam.nl/web10_mp3"};

    radio_selection_menu();
}

/*
 * Function: initialiseer de inhoud van de "song" pagina
 * Parameters: None
 * Returns: None
 */
void song_page_init()
{
    current_page = song_selection_page;

    free(songs);

    char **song_names = read_from_sd();

    songs = (song *)malloc(playlist_size * sizeof(song));

    for (int i = 0; i < playlist_size; i++)
    {
        songs[i] = (song){i, song_names[i]};
    }

    song_selection_menu();
}

void main_menu()
{
    current_page = main_page;

    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "SELECT TYPE");
    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "SD | RADIO | MIC | T");

    // Delete time update task if it exists
    deleteTimeUpdateTask();
}

void input_menu()
{
    current_page = input_page;

    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "INPUT");

    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, "TALK INTO THE MIC");

    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK | x | x | x");
}

void radio_selection_menu()
{
    current_page = radio_selection_page;

    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "SELECT RADIO STATION");

    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, "RADIO: ");

    hd44780_gotoxy(&lcd, 7, 1);
    hd44780_puts(&lcd, stations[station_index].radio_name);

    if ((station_index + 1) < (sizeof(stations) / sizeof(stations[0])))
    {
        hd44780_gotoxy(&lcd, 0, 2);
        hd44780_puts(&lcd, "NEXT: ");

        hd44780_gotoxy(&lcd, 7, 2);
        hd44780_puts(&lcd, stations[station_index + 1].radio_name);
    }

    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK | <- | -> | OK");

    selected_station = stations[station_index];
}

void song_play_menu()
{
    current_page = song_play_page;

    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "PLAYING SONG");
    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK | x | x | x");

    xTaskCreate(init_sd_card_player, "init_sd_card_player", configMINIMAL_STACK_SIZE * 6, NULL, 5, &sd_task_handle);
}

void radio_play_menu()
{
    current_page = radio_play_page;

    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "PLAYING RADIO:");
    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, selected_station.radio_name);
    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK | - | + | x");

    xTaskCreate(start_radio, "start reader", configMINIMAL_STACK_SIZE * 6, selected_station.url, 5, &radio_task_handle);
}

void song_selection_menu()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "SELECT SONG");

    char song_count[20];

    sprintf(song_count, "%d/%d", songs[0].id, playlist_size);

    hd44780_gotoxy(&lcd, 15, 0);
    hd44780_puts(&lcd, song_count);

    hd44780_gotoxy(&lcd, 0, 1);
    hd44780_puts(&lcd, "SONG:");

    hd44780_gotoxy(&lcd, 6, 1);
    char *song_name_1 = format_song_name(songs[0].song_name);
    hd44780_puts(&lcd, song_name_1);

    hd44780_gotoxy(&lcd, 0, 2);
    hd44780_puts(&lcd, "NEXT: ");

    hd44780_gotoxy(&lcd, 6, 2);
    char *song_name_2 = format_song_name(songs[1].song_name);
    hd44780_puts(&lcd, song_name_2);

    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK | <- | -> | OK");
}

void time_menu(const char* Format) {
    deleteTimeUpdateTask();
    current_page = time_menu_page;
    char time_str[64];
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_str, sizeof(time_str), Format, &timeinfo);

    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, time_str);
    hd44780_gotoxy(&lcd, 0, 3);
    hd44780_puts(&lcd, "BACK|  R  | DMY | F");

    // Start time update task
    startTimeUpdateTask(Format);
}

void select_next()
{
    if ((station_index + 1) < (sizeof(stations) / sizeof(stations[0])))
    {
        station_index++;
        // selected_station = stations[station_index];
        radio_selection_menu();
        printf(selected_station.radio_name);
    }
}

void select_previous()
{
    if (station_index > 0)
    {
        station_index--;
        // selected_station = stations[station_index];
        radio_selection_menu();
        printf(selected_station.radio_name);
    }
}

/*
 * Function: zorgt dat de connectie met de radio netjes wordt afgesloten
 * Parameters: None
 * Returns: None
 */

void disconnect_radio()
{
    // delete task
    vTaskDelete(radio_task_handle);

    stop_audio_pipeline();

    radio_selection_menu();
}

void disconnect_sd()
{
    vTaskDelete(sd_task_handle);

    stop_sd_audio_pipeline();

    song_page_init();
}

void unmount_sd(){
    stop_peripherals();
    main_menu();
}

/* 
Functie: format_song_name
Beschrijving: Formaat het nummer, zodat het binnen het scherm van de LCD-past.
Parameters: Het nummer dat geformaat moet worden.
Retourneert: de formatted song die binnen het scherm van de LCD-past.
*/
const char *format_song_name(char *song_name)
{
    char *formatted_name = (char *)malloc(15);
    
    if (formatted_name != NULL) {
        strncpy(formatted_name, song_name, 14);
        formatted_name[14] = '\0';  // Null-terminator
    }

    return formatted_name;
}

void increase_brightness(){
    brightness_leds += 10;

    if (brightness_leds >= 240){
        brightness_leds = 240;
    }

    write_brightness_value(brightness_leds);
}

void decrease_brightness(){
    brightness_leds -= 10;

    if (brightness_leds <= 10){
        brightness_leds = 10;
    }

    write_brightness_value(brightness_leds);
}

/*
 * Function: initialiseer de paginas met de bijbehorende navigatie
 * Parameters: None
 * Returns: None
 */
void initPages()
{
    main_page.set_lcd_text = main_menu;
    main_page.button1 = time_menu_button1_handler;
    main_page.button2 = input_menu;
    main_page.button3 = radio_page_init;
    main_page.button4 = song_page_init;

    input_page.set_lcd_text = input_menu;
    input_page.button1 = NULL;
    input_page.button2 = NULL;
    input_page.button3 = NULL;
    input_page.button4 = main_menu;

    song_selection_page.set_lcd_text = song_selection_menu;
    song_selection_page.button1 = song_play_menu;
    song_selection_page.button2 = NULL;
    song_selection_page.button3 = NULL;
    song_selection_page.button4 = unmount_sd;

    radio_selection_page.set_lcd_text = radio_selection_menu;
    radio_selection_page.button1 = radio_play_menu;
    radio_selection_page.button2 = select_next;
    radio_selection_page.button3 = select_previous;
    radio_selection_page.button4 = main_menu;

    radio_play_page.set_lcd_text = radio_play_menu;
    radio_play_page.button1 = NULL;
    radio_play_page.button2 = increase_brightness;
    radio_play_page.button3 = decrease_brightness;
    radio_play_page.button4 = disconnect_radio;

    song_play_page.set_lcd_text = song_play_menu;
    song_play_page.button1 = NULL;
    song_play_page.button2 = NULL;
    song_play_page.button3 = NULL;
    song_play_page.button4 = disconnect_sd;

    time_menu_page.set_lcd_text = time_menu;
    time_menu_page.button1 = time_menu_button3_handler;
    time_menu_page.button2 = time_menu_button2_handler;
    time_menu_page.button3 = time_menu_button1_handler;
    time_menu_page.button4 = main_menu;

    current_page = main_page;
    current_page.set_lcd_text();
}

/*
 * Function: deze drie methodes zijn de button handlers voor de verschillende tijd formatten 
 * Parameters: None
 * Returns: None
 */
void time_menu_button1_handler() {
    time_menu(day_format);
}
void time_menu_button2_handler() {
    time_menu(day_month_year_format);
}
void time_menu_button3_handler() {
    time_menu(full_format);
}
    

/*
 * Function: Overschrijft de eerste regel op het Time_menu scherm.
 * Parameters: None
 * Returns: None
 */
void update_time_display(const char *Format) {
    char time_str[64];
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_str, sizeof(time_str), Format, &timeinfo);
    
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, time_str);
}

/*
 * Function: Task for updating the time display every second
 * Parameters: None
 * Returns: None
 */
void updateTimeTask(void *Format) {
    const char *time_format = (const char *)Format;
    
    while (1) {
        // Update time display
        update_time_display(time_format);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Update every second
    }
}


/*
 * Function: start de time update task
 * Parameters: None
 * Returns: None
 */
void startTimeUpdateTask(const char *Format) {
    if (time_update_task_handle == NULL) {
        xTaskCreate(updateTimeTask, "TimeUpdateTask", configMINIMAL_STACK_SIZE * 6, (void *)Format, 5, &time_update_task_handle);
    }
}


/*
 * Function: Verwijderd de timeUpdateTask wanneer deze niet meer nodig is
 * Parameters: None
 * Returns: None
 */
void deleteTimeUpdateTask()
{
    if (time_update_task_handle != NULL)
    {
        vTaskDelete(time_update_task_handle);
        time_update_task_handle = NULL;
    }
}

uint16_t previous_value = -1;

/*
 * Function: roept alle initialisaties aan en vraagt de waarde op van de knoppen voor het navigeren door het menu
 * Parameters: None
 * Returns: None
 */
void getButtonValue(void *parameters)
{
    init_lcd();
    initPages();
    while (1)
    {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        if (previous_value != button_value)
        {
            previous_value = button_value;
            ESP_LOGI("hans", "retrieved value: %d", button_value);

            switch (button_value)
            {
            case 1:
                if (current_page.button1 != NULL)
                {
                    current_page.button1();
                }
                break;
            case 2:
                if (current_page.button2 != NULL)
                {
                    current_page.button2();
                }
                break;
            case 4:
                if (current_page.button3 != NULL)
                {
                    current_page.button3();
                }
                break;
            case 8:
                if (current_page.button4 != NULL)
                {
                    current_page.button4();
                }
                break;
            }
        }
        xSemaphoreGive(xMutex);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}