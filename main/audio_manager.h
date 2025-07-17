#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "esp_err.h"
#include "driver/i2s.h"

// Audio manager types
typedef enum {
    AUDIO_STATE_IDLE,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_PAUSED,
    AUDIO_STATE_STOPPED
} audio_state_t;

typedef struct {
    uint32_t sample_rate;
    uint8_t bits_per_sample;
    uint8_t channels;
    audio_state_t state;
    uint8_t volume;
} audio_config_t;

// Function prototypes
esp_err_t audio_manager_init(void);
esp_err_t audio_manager_deinit(void);
esp_err_t audio_manager_start(void);
esp_err_t audio_manager_stop(void);
esp_err_t audio_manager_pause(void);
esp_err_t audio_manager_resume(void);
esp_err_t audio_manager_set_config(uint32_t sample_rate, uint8_t channels);
esp_err_t audio_manager_set_volume(uint8_t volume);
esp_err_t audio_manager_volume_up(void);
esp_err_t audio_manager_volume_down(void);
esp_err_t audio_manager_write_data(const uint8_t *data, size_t size);
audio_state_t audio_manager_get_state(void);
audio_config_t* audio_manager_get_config(void);
uint8_t audio_manager_get_volume(void);

// Audio data callback for A2DP
void audio_data_callback(const uint8_t *data, uint32_t len);

#endif // AUDIO_MANAGER_H
