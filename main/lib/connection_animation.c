#include "lib/connection_animation.h"
#include "lib/lcd_setup.h"

/*
 * Function: animation for starting connection
 * Parameters: None
 * Returns: None
 */
void start_connect_animation()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "starting connection");
    vTaskDelay(750/portTICK_PERIOD_MS);
}

/*
 * Function: Animation when trying to connect
 * Parameters: task parameters
 * Returns: None
 */
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
/*
 * Function: animation when connection is completed
 * Parameters: None
 * Returns: None
 */
void finish_connect_animation()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "Connected :)");
    vTaskDelay(1000/portTICK_PERIOD_MS);
 
}

/*
 * Function: animation for starting ntp time retrieval
 * Parameters: None
 * Returns: None
 */
void start_ntp_connect_animation()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "Starting time sync");
    vTaskDelay(750/portTICK_PERIOD_MS);
}

/*
 * Function: Animation when trying to retrieve/sync time
 * Parameters: task parameters
 * Returns: None
 */
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

/*
 * Function: animation when ntp connection is completed
 * Parameters: None
 * Returns: None
 */
void finish_ntp_connect_animation()
{
    hd44780_clear(&lcd);
    hd44780_gotoxy(&lcd, 0,1);
    hd44780_puts(&lcd, "synced time :)");
    vTaskDelay(1000/portTICK_PERIOD_MS);
 
}
