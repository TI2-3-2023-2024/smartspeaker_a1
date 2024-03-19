#pragma once

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "filter_resample.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "input_key_service.h"
#include "periph_adc_button.h"
#include "board.h"

#include "sdcard_list.h"
#include "sdcard_scan.h"

static const char *SD_TAG = "SDCARD_MP3_CONTROL_EXAMPLE";

extern int playlist_size;

extern char** song_names;

void init_sd_card_player(void *pvParameters);
void stop_sd_audio_pipeline();
char** read_from_sd();
void stop_peripherals();
void play_song(int index);