#include "sd_card_player.h"

audio_pipeline_handle_t pipeline;
audio_element_handle_t i2s_stream_writer, mp3_decoder, fatfs_stream_reader, rsp_handle;
playlist_operator_handle_t sdcard_list_handle = NULL;
esp_periph_set_handle_t set;
audio_event_iface_handle_t evt;
periph_service_handle_t input_ser;
char *url;

static esp_err_t
input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    /* Handle touch pad events
           to start, pause, resume, finish current song and adjust volume
        */
    audio_board_handle_t board_handle = (audio_board_handle_t)ctx;
    int player_volume;
    audio_hal_get_volume(board_handle->audio_hal, &player_volume);

    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE)
    {
        ESP_LOGI(SD_TAG, "[ * ] input key id is %d", (int)evt->data);
        switch ((int)evt->data)
        {
        case INPUT_KEY_USER_ID_PLAY:

            ESP_LOGI(SD_TAG, "[ * ] [Play] input key event");
            audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
            switch (el_state)
            {
            case AEL_STATE_INIT:
                ESP_LOGI(SD_TAG, "[ * ] Starting audio pipeline");
                audio_pipeline_run(pipeline);
                break;
            case AEL_STATE_RUNNING:
                ESP_LOGI(SD_TAG, "[ * ] Pausing audio pipeline");
                audio_pipeline_pause(pipeline);
                break;
            case AEL_STATE_PAUSED:
                ESP_LOGI(SD_TAG, "[ * ] Resuming audio pipeline");
                audio_pipeline_resume(pipeline);
                break;
            default:
                ESP_LOGI(SD_TAG, "[ * ] Not supported state %d", el_state);
            }
            break;
        case INPUT_KEY_USER_ID_SET:
            ESP_LOGI(SD_TAG, "[ * ] [Set] input key event");
            ESP_LOGI(SD_TAG, "[ * ] Stopped, advancing to the next song");
            char *url = NULL;
            audio_pipeline_stop(pipeline);
            audio_pipeline_wait_for_stop(pipeline);
            audio_pipeline_terminate(pipeline);

            sdcard_list_next(sdcard_list_handle, 1, &url);
            ESP_LOGW(SD_TAG, "URL: %s", url);

            audio_element_set_uri(fatfs_stream_reader, url);
            audio_pipeline_reset_ringbuffer(pipeline);
            audio_pipeline_reset_elements(pipeline);
            audio_pipeline_run(pipeline);
            break;
        case INPUT_KEY_USER_ID_VOLUP:
            ESP_LOGI(SD_TAG, "[ * ] [Vol+] input key event");
            player_volume += 10;
            if (player_volume > 100)
            {
                player_volume = 100;
            }
            audio_hal_set_volume(board_handle->audio_hal, player_volume);
            ESP_LOGI(SD_TAG, "[ * ] Volume set to %d %%", player_volume);
            break;
        case INPUT_KEY_USER_ID_VOLDOWN:
            ESP_LOGI(SD_TAG, "[ * ] [Vol-] input key event");
            player_volume -= 10;
            if (player_volume < 0)
            {
                player_volume = 0;
            }
            audio_hal_set_volume(board_handle->audio_hal, player_volume);
            ESP_LOGI(SD_TAG, "[ * ] Volume set to %d %%", player_volume);
            break;
        }
    }

    return ESP_OK;
}

void sdcard_url_save_cb(void *user_data, char *url)
{
    playlist_operator_handle_t sdcard_handle = (playlist_operator_handle_t)user_data;
    esp_err_t ret = sdcard_list_save(sdcard_handle, url);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SD_TAG, "Fail to save sdcard url to sdcard playlist");
    }
}

/*
 * Functie: remove_path
 * Beschrijving: Verwijderd het pad, zodat de titel opgeslagen en gebruikt kan worden.
 * Parameters: File = Totale URL van het bestand, PATH = het deel dat verwijderd moet worden
 * Retourneert: Een char * die de file retoureert met de path er vanaf getrokken.
 */

char *remove_path(char *file, const char *path)
{
    int i = 0;
    while (file[i] == path[i])
    {
        i++;
    }
    char *filename = file + i;
    return filename;
}

void init_sd_card_player(void *pVParameters)
{
    ESP_LOGI(SD_TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    ESP_LOGI(SD_TAG, "[ 3 ] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    //periph_service_set_callback(input_ser, input_key_service_cb, (void *)board_handle);

    ESP_LOGI(SD_TAG, "[4.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(SD_TAG, "[4.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    i2s_stream_set_clk(i2s_stream_writer, 48000, 16, 2);

    ESP_LOGI(SD_TAG, "[4.2] Create mp3 decoder to decode mp3 file");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

    /* ZL38063 audio chip on board of ESP32-LyraTD-MSC does not support 44.1 kHz sampling frequency,
       so resample filter has been added to convert audio data to other rates accepted by the chip.
       You can resample the data to 16 kHz or 48 kHz.
    */
    ESP_LOGI(SD_TAG, "[4.3] Create resample filter");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_handle = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(SD_TAG, "[4.4] Create fatfs stream to read data from sdcard");
    url = NULL;
    sdcard_list_current(sdcard_list_handle, &url);
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);
    audio_element_set_uri(fatfs_stream_reader, url);

    ESP_LOGI(SD_TAG, "[4.5] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, fatfs_stream_reader, "file");
    audio_pipeline_register(pipeline, mp3_decoder, "mp3");
    audio_pipeline_register(pipeline, rsp_handle, "filter");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(SD_TAG, "[4.6] Link it together [sdcard]-->fatfs_stream-->mp3_decoder-->resample-->i2s_stream-->[codec_chip]");
    const char *link_tag[4] = {"file", "mp3", "filter", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 4);

    ESP_LOGI(SD_TAG, "[5.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(SD_TAG, "[5.1] Listen for all pipeline events");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGW(SD_TAG, "[ 6 ] Press the keys to control music player:");
    ESP_LOGW(SD_TAG, "      [Play] to start, pause and resume, [Set] next song.");
    ESP_LOGW(SD_TAG, "      [Vol-] or [Vol+] to adjust volume.");

    while (1)
    {
        /* Handle event interface messages from pipeline
           to set music info and to advance to the next song
        */
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SD_TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT)
        {
            // Set music info for a new song to be played
            if (msg.source == (void *)mp3_decoder && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO)
            {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(mp3_decoder, &music_info);
                ESP_LOGI(SD_TAG, "[ * ] Received music info from mp3 decoder, sample_rates=%d, bits=%d, ch=%d",
                         music_info.sample_rates, music_info.bits, music_info.channels);
                audio_element_setinfo(i2s_stream_writer, &music_info);
                rsp_filter_set_src_info(rsp_handle, music_info.sample_rates, music_info.channels);
                continue;
            }
            // Advance to the next song when previous finishes
            // if (msg.source == (void *)i2s_stream_writer && msg.cmd == AEL_MSG_CMD_REPORT_STATUS)
            // {
            //     audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
            //     if (el_state == AEL_STATE_FINISHED)
            //     {
            //         ESP_LOGI(SD_TAG, "[ * ] Finished, advancing to the next song");
            //         sdcard_list_next(sdcard_list_handle, 1, &url);
            //         ESP_LOGW(SD_TAG, "URL: %s", url);
            //         /* In previous versions, audio_pipeline_terminal() was called here. It will close all the element task and when we use
            //          * the pipeline next time, all the tasks should be restarted again. It wastes too much time when we switch to another music.
            //          * So we use another method to achieve this as below.
            //          */
            //         audio_element_set_uri(fatfs_stream_reader, url);
            //         audio_pipeline_reset_ringbuffer(pipeline);
            //         audio_pipeline_reset_elements(pipeline);
            //         audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
            //         audio_pipeline_run(pipeline);
            //     }
            //     continue;
            // }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    stop_sd_audio_pipeline();
}

char** read_from_sd()
{
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(SD_TAG, ESP_LOG_INFO);

    ESP_LOGI(SD_TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(SD_TAG, "[1.1] Initialize and start peripherals");
    audio_board_key_init(set);
    audio_board_sdcard_init(set, SD_MODE_1_LINE);

    ESP_LOGI(SD_TAG, "[1.2] Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle);
    sdcard_scan(sdcard_url_save_cb, "/sdcard/songs", 0, (const char *[]){"mp3"}, 1, sdcard_list_handle);

    ESP_LOGI(SD_TAG, "[1.3] PRINTING ALL THE SONGS");
    url = NULL;

    playlist_operation_t operation;
    playlist_handle_t playlist = sdcard_list_handle->playlist;

    esp_err_t err = sdcard_list_handle->get_operation(&operation);
    if (err != ESP_OK)
    {
        ESP_LOGE(SD_TAG, "Failed to get playlist operation");
    }

    playlist_size = sdcard_list_get_url_num(&playlist);
    song_names = (char **)malloc(playlist_size * sizeof(char *));

    ESP_LOGI(SD_TAG, "[1.4.1] URL COUNT %d", playlist_size);

    for (int i = 0; i < playlist_size; i++)
    {
        if (i == 0)
        {
            sdcard_list_current(sdcard_list_handle, &url); // url is default NULL, so song 1 on the SD card needs to be included as well
        }
        else
            sdcard_list_next(sdcard_list_handle, 1, &url); // rest of the songs gets added

        char *song_name = strdup(remove_path(url, "file://sdcard/songs/"));

        if (song_name != NULL)
        {
            ESP_LOGI(SD_TAG, "[ SONG %d ]: %s", i, song_name);
            song_names[i] = song_name;
        }
        else
        {
            ESP_LOGE(SD_TAG, "Failed to allocate memory for song name");
            break;
        }
    }

    // For-loop to check if the song array is filled it with titles of songs
    for (int i = 0; i < playlist_size; i++)
    {
        char *song_name = song_names[i];
        ESP_LOGE(SD_TAG, "[ SONG %d ]: %s", i, song_name);
    }

    return song_names;
}

void stop_sd_audio_pipeline()
{
    ESP_LOGI(SD_TAG, "[ 7 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, mp3_decoder);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, rsp_handle);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    sdcard_list_destroy(sdcard_list_handle);
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(mp3_decoder);
    audio_element_deinit(rsp_handle);
    periph_service_destroy(input_ser);
    esp_periph_set_destroy(set);
}

void stop_peripherals(){
    esp_periph_set_stop_all(set);
    sdcard_list_destroy(sdcard_list_handle);
    esp_periph_set_destroy(set);
}


/*
 * Functie: play_song
 * Beschrijving: start de audio pipeline en speelt het nummer af van het meegegeven index
 * Parameters: index = plaats van nummer in de playlist om af te spelen
 * Retourneert: geen
 */
void play_song(int index){
    sdcard_list_next(sdcard_list_handle, index + 1, &url);
    audio_element_set_uri(fatfs_stream_reader, url);
    audio_pipeline_reset_ringbuffer(pipeline);
    audio_pipeline_reset_elements(pipeline);
    audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
    audio_pipeline_run(pipeline);
}