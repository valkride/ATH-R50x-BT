#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_a2dp_api.h"

#include "config.h"
#include "audio_manager.h"

static const char *TAG = "AUDIO_MANAGER";

// Audio manager state
static audio_config_t audio_config = {
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = AUDIO_BITS_PER_SAMPLE,
    .channels = AUDIO_CHANNELS,
    .state = AUDIO_STATE_IDLE,
    .volume = AUDIO_VOLUME_DEFAULT
};

static bool audio_initialized = false;
static RingbufHandle_t audio_ringbuf = NULL;
static TaskHandle_t audio_task_handle = NULL;

// Volume control
static float volume_table[AUDIO_VOLUME_STEPS + 1];

// Function prototypes
static void audio_task(void *pvParameters);
static void audio_init_volume_table(void);
static esp_err_t audio_init_i2s(void);
static esp_err_t audio_deinit_i2s(void);
static void audio_apply_volume(int16_t *data, size_t samples);
static void audio_fade_volume(int16_t *data, size_t samples, float start_vol, float end_vol);

esp_err_t audio_manager_init(void)
{
    esp_err_t ret;
    
    DEBUG_INFO("Initializing audio manager");
    
    // Initialize volume table
    audio_init_volume_table();
    
    // Initialize I2S
    ret = audio_init_i2s();
    if (ret != ESP_OK) {
        DEBUG_ERROR("I2S initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create audio ring buffer
    audio_ringbuf = xRingbufferCreate(AUDIO_BUFFER_SIZE * 4, RINGBUF_TYPE_BYTEBUF);
    if (audio_ringbuf == NULL) {
        DEBUG_ERROR("Failed to create audio ring buffer");
        audio_deinit_i2s();
        return ESP_ERR_NO_MEM;
    }
    
    // Create audio processing task
    if (xTaskCreate(audio_task, "audio_task", 4096, NULL, 10, &audio_task_handle) != pdPASS) {
        DEBUG_ERROR("Failed to create audio task");
        vRingbufferDelete(audio_ringbuf);
        audio_deinit_i2s();
        return ESP_ERR_NO_MEM;
    }
    
    // Register A2DP audio data callback
    esp_a2dp_sink_register_data_callback(audio_data_callback);
    
    audio_initialized = true;
    audio_config.state = AUDIO_STATE_IDLE;
    
    DEBUG_INFO("Audio manager initialized successfully");
    return ESP_OK;
}

esp_err_t audio_manager_deinit(void)
{
    if (!audio_initialized) {
        return ESP_OK;
    }
    
    DEBUG_INFO("Deinitializing audio manager");
    
    // Stop audio processing
    audio_manager_stop();
    
    // Delete audio task
    if (audio_task_handle) {
        vTaskDelete(audio_task_handle);
        audio_task_handle = NULL;
    }
    
    // Delete ring buffer
    if (audio_ringbuf) {
        vRingbufferDelete(audio_ringbuf);
        audio_ringbuf = NULL;
    }
    
    // Deinitialize I2S
    audio_deinit_i2s();
    
    audio_initialized = false;
    audio_config.state = AUDIO_STATE_IDLE;
    
    DEBUG_INFO("Audio manager deinitialized");
    return ESP_OK;
}

esp_err_t audio_manager_start(void)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Starting audio playback");
    
    esp_err_t ret = i2s_start(I2S_NUM_0);
    if (ret != ESP_OK) {
        DEBUG_ERROR("I2S start failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    audio_config.state = AUDIO_STATE_PLAYING;
    return ESP_OK;
}

esp_err_t audio_manager_stop(void)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Stopping audio playback");
    
    esp_err_t ret = i2s_stop(I2S_NUM_0);
    if (ret != ESP_OK) {
        DEBUG_ERROR("I2S stop failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    audio_config.state = AUDIO_STATE_STOPPED;
    return ESP_OK;
}

esp_err_t audio_manager_pause(void)
{
    if (!audio_initialized || audio_config.state != AUDIO_STATE_PLAYING) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Pausing audio playback");
    
    audio_config.state = AUDIO_STATE_PAUSED;
    return ESP_OK;
}

esp_err_t audio_manager_resume(void)
{
    if (!audio_initialized || audio_config.state != AUDIO_STATE_PAUSED) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Resuming audio playback");
    
    audio_config.state = AUDIO_STATE_PLAYING;
    return ESP_OK;
}

esp_err_t audio_manager_set_config(uint32_t sample_rate, uint8_t channels)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Setting audio config: sample_rate=%d, channels=%d", sample_rate, channels);
    
    bool was_playing = (audio_config.state == AUDIO_STATE_PLAYING);
    
    // Stop I2S if playing
    if (was_playing) {
        i2s_stop(I2S_NUM_0);
    }
    
    // Update configuration
    audio_config.sample_rate = sample_rate;
    audio_config.channels = channels;
    
    // Reconfigure I2S
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = audio_config.sample_rate,
        .bits_per_sample = audio_config.bits_per_sample,
        .channel_format = (audio_config.channels == 1) ? I2S_CHANNEL_FMT_ONLY_LEFT : I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    esp_err_t ret = i2s_set_clk(I2S_NUM_0, audio_config.sample_rate, audio_config.bits_per_sample, 
                                (audio_config.channels == 1) ? I2S_CHANNEL_MONO : I2S_CHANNEL_STEREO);
    if (ret != ESP_OK) {
        DEBUG_ERROR("I2S set clock failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Restart I2S if was playing
    if (was_playing) {
        i2s_start(I2S_NUM_0);
    }
    
    DEBUG_INFO("Audio config updated successfully");
    return ESP_OK;
}

esp_err_t audio_manager_set_volume(uint8_t volume)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (volume > AUDIO_VOLUME_STEPS) {
        volume = AUDIO_VOLUME_STEPS;
    }
    
    DEBUG_INFO("Setting audio volume to %d", volume);
    
    audio_config.volume = volume;
    return ESP_OK;
}

esp_err_t audio_manager_volume_up(void)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (audio_config.volume < AUDIO_VOLUME_STEPS) {
        audio_config.volume++;
        DEBUG_INFO("Volume up: %d", audio_config.volume);
    }
    
    return ESP_OK;
}

esp_err_t audio_manager_volume_down(void)
{
    if (!audio_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (audio_config.volume > 0) {
        audio_config.volume--;
        DEBUG_INFO("Volume down: %d", audio_config.volume);
    }
    
    return ESP_OK;
}

esp_err_t audio_manager_write_data(const uint8_t *data, size_t size)
{
    if (!audio_initialized || !audio_ringbuf) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Write data to ring buffer
    if (xRingbufferSend(audio_ringbuf, data, size, pdMS_TO_TICKS(100)) != pdTRUE) {
        DEBUG_WARN("Audio ring buffer full, dropping data");
        return ESP_ERR_NO_MEM;
    }
    
    return ESP_OK;
}

// A2DP audio data callback
void audio_data_callback(const uint8_t *data, uint32_t len)
{
    if (audio_initialized && audio_ringbuf) {
        audio_manager_write_data(data, len);
    }
}

// Audio processing task
static void audio_task(void *pvParameters)
{
    size_t item_size;
    uint8_t *data;
    size_t bytes_written;
    
    DEBUG_INFO("Audio task started");
    
    while (1) {
        // Receive data from ring buffer
        data = (uint8_t *)xRingbufferReceive(audio_ringbuf, &item_size, pdMS_TO_TICKS(100));
        
        if (data != NULL) {
            // Apply volume control
            if (audio_config.volume < AUDIO_VOLUME_STEPS) {
                audio_apply_volume((int16_t *)data, item_size / sizeof(int16_t));
            }
            
            // Write to I2S if playing
            if (audio_config.state == AUDIO_STATE_PLAYING) {
                esp_err_t ret = i2s_write(I2S_NUM_0, data, item_size, &bytes_written, pdMS_TO_TICKS(100));
                if (ret != ESP_OK) {
                    DEBUG_WARN("I2S write failed: %s", esp_err_to_name(ret));
                }
            }
            
            // Return the item
            vRingbufferReturnItem(audio_ringbuf, (void *)data);
        }
        
        // Small delay to prevent task from hogging CPU
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Initialize I2S
static esp_err_t audio_init_i2s(void)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = audio_config.sample_rate,
        .bits_per_sample = audio_config.bits_per_sample,
        .channel_format = (audio_config.channels == 1) ? I2S_CHANNEL_FMT_ONLY_LEFT : I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = GPIO_I2S_BCK,
        .ws_io_num = GPIO_I2S_WS,
        .data_out_num = GPIO_I2S_DATA,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        DEBUG_ERROR("I2S driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (ret != ESP_OK) {
        DEBUG_ERROR("I2S set pin failed: %s", esp_err_to_name(ret));
        i2s_driver_uninstall(I2S_NUM_0);
        return ret;
    }
    
    DEBUG_INFO("I2S initialized: sample_rate=%d, bits=%d, channels=%d", 
               audio_config.sample_rate, audio_config.bits_per_sample, audio_config.channels);
    
    return ESP_OK;
}

// Deinitialize I2S
static esp_err_t audio_deinit_i2s(void)
{
    return i2s_driver_uninstall(I2S_NUM_0);
}

// Initialize volume table with logarithmic scaling
static void audio_init_volume_table(void)
{
    for (int i = 0; i <= AUDIO_VOLUME_STEPS; i++) {
        if (i == 0) {
            volume_table[i] = 0.0f;
        } else {
            // Logarithmic volume scaling
            volume_table[i] = powf(10.0f, (float)(i - AUDIO_VOLUME_STEPS) * 3.0f / AUDIO_VOLUME_STEPS);
        }
    }
}

// Apply volume control to audio data
static void audio_apply_volume(int16_t *data, size_t samples)
{
    float volume = volume_table[audio_config.volume];
    
    for (size_t i = 0; i < samples; i++) {
        int32_t sample = (int32_t)data[i] * volume;
        
        // Clamp to prevent overflow
        if (sample > 32767) {
            sample = 32767;
        } else if (sample < -32768) {
            sample = -32768;
        }
        
        data[i] = (int16_t)sample;
    }
}

// Fade volume for smooth transitions
static void audio_fade_volume(int16_t *data, size_t samples, float start_vol, float end_vol)
{
    float vol_step = (end_vol - start_vol) / samples;
    
    for (size_t i = 0; i < samples; i++) {
        float current_vol = start_vol + (vol_step * i);
        int32_t sample = (int32_t)data[i] * current_vol;
        
        // Clamp to prevent overflow
        if (sample > 32767) {
            sample = 32767;
        } else if (sample < -32768) {
            sample = -32768;
        }
        
        data[i] = (int16_t)sample;
    }
}

// Getter functions
audio_state_t audio_manager_get_state(void)
{
    return audio_config.state;
}

audio_config_t* audio_manager_get_config(void)
{
    return &audio_config;
}

uint8_t audio_manager_get_volume(void)
{
    return audio_config.volume;
}
