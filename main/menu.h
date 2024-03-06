#pragma once

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>
#include <hd44780.h>
#include <pcf8574.h>
#include <string.h>


typedef struct
{
    int id;
    char *song_name;
} song;

void input_menu();
void main_menu();
void song_selection_menu(song songs[], size_t size);

void init_lcd(void *pvParameters);