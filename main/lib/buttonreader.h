#ifndef BUTTONREADER_H
#define BUTTONREADER_H

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mcp23x17.h>
#include <driver/gpio.h>
#include <string.h>
#include <esp_log.h>



void start_reader();



#endif