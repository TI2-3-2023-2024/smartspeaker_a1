#include "lib/connection_animation.h"

static i2c_dev_t pcf8574;
static hd44780_t lcd;

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}
void init_lcd_connection()
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
}

void start_connect_animation()
{
    init_lcd_connection();
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "starting connection");
    vTaskDelay(750/portTICK_PERIOD_MS);
}
    
void establish_connect_animation_task(void * pvParameters)
{
    static const char* dots[3] = {".  ",".. ","..."};
    int dotnumber = 0;
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "Connecting");
    while(1)
    {
        hd44780_gotoxy(&lcd,10,1);
        hd44780_puts(&lcd, dots[dotnumber]);
        dotnumber = (dotnumber + 1) % 3;
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
   
}

void finish_connect_animation()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "Connected :)");
    vTaskDelay(1000/portTICK_PERIOD_MS);
 
}

void start_ntp_connect_animation()
{
    init_lcd_connection();
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "Starting time sync");
    vTaskDelay(750/portTICK_PERIOD_MS);
}

void establish_ntp_connect_animation_task(void * pvParameters)
{
    static const char* dots[3] = {".  ",".. ","..."};
    int dotnumber = 0;
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "Retrieving time");
    while(1)
    {
        hd44780_gotoxy(&lcd,15,1);
        hd44780_puts(&lcd, dots[dotnumber]);
        dotnumber = (dotnumber + 1) % 3;
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
   
}

void finish_ntp_connect_animation()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "synced time :)");
    vTaskDelay(1000/portTICK_PERIOD_MS);
 
}
