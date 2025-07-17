#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include "esp_err.h"
#include "esp_bt_defs.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

// Bluetooth manager types
typedef enum {
    BT_STATE_IDLE,
    BT_STATE_DISCOVERABLE,
    BT_STATE_CONNECTING,
    BT_STATE_CONNECTED,
    BT_STATE_PLAYING,
    BT_STATE_PAUSED
} bt_state_t;

typedef struct {
    esp_bd_addr_t remote_addr;
    char device_name[32];
    bool is_connected;
    bt_state_t state;
} bt_device_info_t;

// System event types
typedef enum {
    SYSTEM_EVENT_BT_CONNECTED,
    SYSTEM_EVENT_BT_DISCONNECTED,
    SYSTEM_EVENT_AUDIO_PLAY,
    SYSTEM_EVENT_AUDIO_PAUSE,
    SYSTEM_EVENT_BUTTON_PLAY,
    SYSTEM_EVENT_BUTTON_VOL_UP,
    SYSTEM_EVENT_BUTTON_VOL_DOWN,
    SYSTEM_EVENT_POWER_LOW,
    SYSTEM_EVENT_POWER_SLEEP,
    SYSTEM_EVENT_POWER_WAKE
} system_event_type_t;

typedef struct {
    system_event_type_t type;
    void *data;
} system_event_t;

// Global system event queue
extern QueueHandle_t system_event_queue;

// Function prototypes
esp_err_t bluetooth_manager_init(void);
esp_err_t bluetooth_manager_deinit(void);
esp_err_t bluetooth_manager_start_discovery(void);
esp_err_t bluetooth_manager_stop_discovery(void);
esp_err_t bluetooth_manager_connect(esp_bd_addr_t remote_addr);
esp_err_t bluetooth_manager_disconnect(void);
esp_err_t bluetooth_manager_toggle_play(void);
esp_err_t bluetooth_manager_next_track(void);
esp_err_t bluetooth_manager_prev_track(void);
esp_err_t bluetooth_manager_volume_up(void);
esp_err_t bluetooth_manager_volume_down(void);
bt_state_t bluetooth_manager_get_state(void);
bt_device_info_t* bluetooth_manager_get_device_info(void);
const char* bluetooth_manager_get_device_name(void);
bool bluetooth_manager_is_connected(void);

#endif // BLUETOOTH_MANAGER_H
