#include "lib/connection_animation.h"
#include "lib/lcd_setup.h"


void start_connect_animation()
{
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
