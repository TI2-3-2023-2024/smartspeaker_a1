#pragma once

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>
#include <hd44780.h>
#include <pcf8574.h>
#include <string.h>
#include <esp_log.h>


typedef struct
{
    int id;
    char *song_name;
} song;

typedef struct
{
    int id;
    char *radio_name;
    char *url;
} radio_station;

typedef struct
{
  void (*set_lcd_text)(); 

  void (*button1)(); 
  void (*button2)(); 
  void (*button3)(); 
  void (*button4)(); 
} menu_page;

void input_menu();
void main_menu();
void song_selection_menu();
void radio_selection_menu();
void update_time_display();
void updateTimeTask(void *parameters);
void startTimeUpdateTask();
void deleteTimeUpdateTask();

void time_menu_button1_handler();
void time_menu_button2_handler();
void time_menu_button3_handler();

void init_lcd();
void getButtonValue();
const char *format_song_name(char *song_name);

void decrease_brightness();
void increase_brightness();