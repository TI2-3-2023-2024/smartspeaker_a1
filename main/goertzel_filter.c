/*
Bestandsnaam: main.c
Auteur: Niels, Ruben en Thijme
Datum: 28-2-2024
Beschrijving: code om lcd menu aan te sturen voor sprint demo 1
*/

#include <inttypes.h>

#include "menu.h"
#include "i2c_display.h"
#include "lib\buttonreader.h"
#include "sharedvariable.h"

#include "lib/internet_radio.h"
#include "lib/ntp.h"
#include "lib/wifi_setup.h"

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
#include "raw_stream.h"
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

#include <math.h>
#include "filter_resample.h"
#include "lib/goertzel.h"

#define GOERTZEL_SAMPLE_RATE_HZ 8000 // Sample rate in [Hz]
#define GOERTZEL_FRAME_LENGTH_MS 100 // Block length in [ms]

#define GOERTZEL_BUFFER_LENGTH (GOERTZEL_FRAME_LENGTH_MS * GOERTZEL_SAMPLE_RATE_HZ / 10000) // Buffer length in samples

#define GOERTZEL_DETECTION_THRESHOLD 10.0f // Detect a tone when log magnitude is above this value

#define AUDIO_SAMPLE_RATE 48000 // Audio capture sample rate [Hz]

static const int GOERTZEL_DETECT_FREQS[] = {
    50,
    70,
    200,
    400,
    1000,
    3000,
    5000,
    15000
    };

#define GOERTZEL_NR_FREQS ((sizeof GOERTZEL_DETECT_FREQS) / (sizeof GOERTZEL_DETECT_FREQS[0]))

SemaphoreHandle_t xMutex;

audio_pipeline_handle_t pipeline;
audio_pipeline_handle_t pipeline_reader;
audio_element_handle_t i2s_stream_reader, raw_reader, mp3_decoder, fatfs_stream_reader, rsp_handle, rsp_handle_read;
playlist_operator_handle_t sdcard_list_handle2 = NULL;

goertzel_filter_cfg_t filters_cfg[GOERTZEL_NR_FREQS];
goertzel_filter_data_t filters_data[GOERTZEL_NR_FREQS];

i2s_stream_cfg_t i2s_cfg;

static audio_element_handle_t create_i2s_stream(int sample_rate, audio_stream_type_t type)
{
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = type;
    i2s_cfg.i2s_config.sample_rate = sample_rate;
    audio_element_handle_t i2s_stream = i2s_stream_init(&i2s_cfg);
    return i2s_stream;
}

static audio_element_handle_t create_resample_filter(
    int source_rate, int source_channels, int dest_rate, int dest_channels)
{
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = source_rate;
    rsp_cfg.src_ch = source_channels;
    rsp_cfg.dest_rate = dest_rate;
    rsp_cfg.dest_ch = dest_channels;
    audio_element_handle_t filter = rsp_filter_init(&rsp_cfg);
    return filter;
}

static audio_element_handle_t create_raw_stream()
{
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 8 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    audio_element_handle_t raw_reader = raw_stream_init(&raw_cfg);
    return raw_reader;
}

static audio_pipeline_handle_t create_pipeline()
{
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audio_pipeline_handle_t pipeline = audio_pipeline_init(&pipeline_cfg);
    return pipeline;
}

/**
 * Determine if a frequency was detected or not, based on the magnitude that the
 * Goertzel filter calculated
 * Use a logarithm for the magnitude
 */
static void detect_freq(int target_freq, float magnitude)
{
    float logMagnitude = 10.0f * log10f(magnitude);
    if (logMagnitude > GOERTZEL_DETECTION_THRESHOLD)
    {
        ESP_LOGI(TAG, "Detection at frequency %d Hz (magnitude %.2f, log magnitude %.2f)", target_freq, magnitude, logMagnitude);
        vTaskDelay(10);
    }
}
void sdcard_url_save_cb2(void *user_data, char *url)
{
    playlist_operator_handle_t sdcard_handle = (playlist_operator_handle_t)user_data;
    printf("url: %s", url);
    esp_err_t ret = sdcard_list_save(sdcard_handle, url);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Fail to save sdcard url to sdcard playlist");
    }
}

void goertzel_reader()
{
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "Create raw sample buffer");
    int16_t *raw_buffer = (int16_t *)malloc((GOERTZEL_BUFFER_LENGTH * sizeof(int16_t)));
    if (raw_buffer == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation for raw sample buffer failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Setup Goertzel detection filters");
    for (int f = 0; f < GOERTZEL_NR_FREQS; f++)
    {
        filters_cfg[f].sample_rate = GOERTZEL_SAMPLE_RATE_HZ;
        filters_cfg[f].target_freq = GOERTZEL_DETECT_FREQS[f];
        filters_cfg[f].buffer_length = GOERTZEL_BUFFER_LENGTH;
        esp_err_t error = goertzel_filter_setup(&filters_data[f], &filters_cfg[f]);
        ESP_ERROR_CHECK(error);
    }

    ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[1.1] Initialize and start peripherals");
    audio_board_sdcard_init(set, SD_MODE_1_LINE);

    ESP_LOGI(TAG, "[1.2] Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle2);
    sdcard_scan(sdcard_url_save_cb2, "/sdcard/songs", 0, (const char *[]){"mp3"}, 1, sdcard_list_handle2);
    sdcard_list_show(sdcard_list_handle2);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    //note is anders
    ESP_LOGI(TAG, "[4.0.1] Create reader pipeline for playback");
    pipeline_reader = create_pipeline();

    i2s_stream_reader = create_i2s_stream(AUDIO_SAMPLE_RATE, AUDIO_STREAM_READER);

    ESP_LOGI(TAG, "[4.2] Create mp3 decoder to decode mp3 file");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

    /* ZL38063 audio chip on board of ESP32-LyraTD-MSC does not support 44.1 kHz sampling frequency,
       so resample filter has been added to convert audio data to other rates accepted by the chip.
       You can resample the data to 16 kHz or 48 kHz.
    */
    ESP_LOGI(TAG, "[4.3] Create resample filter");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_handle = rsp_filter_init(&rsp_cfg);
    rsp_handle_read = create_resample_filter(AUDIO_SAMPLE_RATE, 2, GOERTZEL_SAMPLE_RATE_HZ, 1);
    raw_reader = create_raw_stream();

    ESP_LOGI(TAG, "[4.4] Create fatfs stream to read data from sdcard");
    char *url = NULL;
    sdcard_list_current(sdcard_list_handle2, &url);
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);
    audio_element_set_uri(fatfs_stream_reader, url);

    audio_pipeline_register(pipeline_reader, fatfs_stream_reader, "file_r");
    audio_pipeline_register(pipeline_reader, mp3_decoder, "mp3_r");
    audio_pipeline_register(pipeline_reader, rsp_handle_read, "rsp_filter");
    audio_pipeline_register(pipeline_reader, i2s_stream_reader, "i2sreader");
    audio_pipeline_register(pipeline_reader, raw_reader, "raw");

    ESP_LOGI(TAG, "[4.6] Link it together [sdcard]-->fatfs_stream-->mp3_decoder-->resample-->i2s_stream-->[codec_chip]");

    const char *link_tag2[5] = {"file_r", "mp3_r", "i2sreader", "rsp_filter", "raw"};
    audio_pipeline_link(pipeline_reader, &link_tag2[0], 5);

    ESP_LOGI(TAG, "[5.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[5.2] Start pipeline");
    audio_pipeline_run(pipeline_reader);

    while (1)
    {

        raw_stream_read(raw_reader, (char *)raw_buffer, GOERTZEL_BUFFER_LENGTH * sizeof(int16_t));
        for (int f = 0; f < GOERTZEL_NR_FREQS; f++)
        {
            float magnitude;

            esp_err_t error = goertzel_filter_process(&filters_data[f], raw_buffer, GOERTZEL_BUFFER_LENGTH);
            ESP_ERROR_CHECK(error);

            goertzel_filter_new_magnitude(&filters_data[f], &magnitude);
            detect_freq(filters_cfg[f].target_freq, magnitude);
        }
    }

    ESP_LOGI(TAG, "Deallocate raw sample buffer memory");
    free(raw_buffer);

    audio_pipeline_stop(pipeline_reader);
    audio_pipeline_wait_for_stop(pipeline_reader);
    audio_pipeline_terminate(pipeline_reader);

    audio_pipeline_unregister(pipeline_reader, mp3_decoder);
    audio_pipeline_unregister(pipeline_reader, i2s_stream_reader);
    audio_pipeline_unregister(pipeline_reader, rsp_handle_read);
    audio_pipeline_unregister(pipeline_reader, raw_reader);

    audio_pipeline_deinit(pipeline_reader);

    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(rsp_handle_read);
    audio_element_deinit(raw_reader);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    sdcard_list_destroy(sdcard_list_handle2);
    audio_pipeline_deinit(pipeline_reader);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(mp3_decoder);
    audio_element_deinit(rsp_handle);
    esp_periph_set_destroy(set);

    vTaskDelete(NULL);
}