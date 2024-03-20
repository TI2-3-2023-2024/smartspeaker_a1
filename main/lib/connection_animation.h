#pragma once

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>
#include <hd44780.h>
#include <pcf8574.h>
#include <string.h>
#include <esp_log.h>


void start_connect_animation();
void establish_connect_animation_task(void * pvParameters);
void finish_connect_animation();

void start_ntp_connect_animation();
void establish_ntp_connect_animation_task(void * pvParameters);
void finish_ntp_connect_animation();