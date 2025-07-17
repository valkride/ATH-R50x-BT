#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "nvs.h"

#include "config.h"
#include "bluetooth_manager.h"
#include "audio_manager.h"

static const char *TAG = "BT_MANAGER";

// Global system event queue
QueueHandle_t system_event_queue;

// Bluetooth manager state
static bt_state_t bt_state = BT_STATE_IDLE;
static bt_device_info_t device_info = {0};
static bool bt_initialized = false;

// A2DP and AVRC handles
static uint16_t a2dp_handle = 0;
static uint8_t avrc_handle = 0;

// Function prototypes
static void bt_gap_event_handler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static void bt_a2dp_event_handler(esp_a2dp_cb_event_t event, esp_a2dp_cb_param_t *param);
static void bt_avrc_event_handler(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
static void bt_send_system_event(system_event_type_t type, void *data);
static esp_err_t bt_load_paired_devices(void);
static esp_err_t bt_save_paired_device(esp_bd_addr_t addr);

esp_err_t bluetooth_manager_init(void)
{
    esp_err_t ret;
    
    DEBUG_INFO("Initializing Bluetooth manager");
    
    // Create system event queue
    system_event_queue = xQueueCreate(SYSTEM_QUEUE_SIZE, sizeof(system_event_t));
    if (system_event_queue == NULL) {
        DEBUG_ERROR("Failed to create system event queue");
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Bluetooth controller init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Enable Bluetooth controller
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Initialize Bluetooth stack
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        DEBUG_ERROR("Bluetooth stack init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Enable Bluetooth stack
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        DEBUG_ERROR("Bluetooth stack enable failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set device name
    ret = esp_bt_dev_set_device_name(DEVICE_NAME);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Set device name failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set scan mode (discoverable and connectable)
    ret = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Set scan mode failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register GAP callback
    ret = esp_bt_gap_register_callback(bt_gap_event_handler);
    if (ret != ESP_OK) {
        DEBUG_ERROR("GAP callback registration failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Initialize A2DP sink
    ret = esp_a2dp_sink_init();
    if (ret != ESP_OK) {
        DEBUG_ERROR("A2DP sink init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register A2DP callback
    ret = esp_a2dp_register_callback(bt_a2dp_event_handler);
    if (ret != ESP_OK) {
        DEBUG_ERROR("A2DP callback registration failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Initialize AVRC controller
    ret = esp_avrc_ct_init();
    if (ret != ESP_OK) {
        DEBUG_ERROR("AVRC controller init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register AVRC callback
    ret = esp_avrc_ct_register_callback(bt_avrc_event_handler);
    if (ret != ESP_OK) {
        DEBUG_ERROR("AVRC callback registration failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Load paired devices from NVS
    bt_load_paired_devices();
    
    bt_initialized = true;
    bt_state = BT_STATE_DISCOVERABLE;
    
    DEBUG_INFO("Bluetooth manager initialized successfully");
    return ESP_OK;
}

esp_err_t bluetooth_manager_deinit(void)
{
    if (!bt_initialized) {
        return ESP_OK;
    }
    
    DEBUG_INFO("Deinitializing Bluetooth manager");
    
    // Disconnect if connected
    if (bt_state == BT_STATE_CONNECTED || bt_state == BT_STATE_PLAYING) {
        bluetooth_manager_disconnect();
    }
    
    // Deinitialize AVRC
    esp_avrc_ct_deinit();
    
    // Deinitialize A2DP
    esp_a2dp_sink_deinit();
    
    // Disable Bluetooth stack
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    
    // Disable Bluetooth controller
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    
    // Clean up
    if (system_event_queue) {
        vQueueDelete(system_event_queue);
        system_event_queue = NULL;
    }
    
    bt_initialized = false;
    bt_state = BT_STATE_IDLE;
    
    DEBUG_INFO("Bluetooth manager deinitialized");
    return ESP_OK;
}

esp_err_t bluetooth_manager_start_discovery(void)
{
    if (!bt_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Starting device discovery");
    
    esp_err_t ret = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Start discovery failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t bluetooth_manager_stop_discovery(void)
{
    if (!bt_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Stopping device discovery");
    
    esp_err_t ret = esp_bt_gap_cancel_discovery();
    if (ret != ESP_OK) {
        DEBUG_ERROR("Stop discovery failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t bluetooth_manager_connect(esp_bd_addr_t remote_addr)
{
    if (!bt_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Connecting to device");
    
    memcpy(device_info.remote_addr, remote_addr, sizeof(esp_bd_addr_t));
    bt_state = BT_STATE_CONNECTING;
    
    esp_err_t ret = esp_a2dp_sink_connect(remote_addr);
    if (ret != ESP_OK) {
        DEBUG_ERROR("A2DP connect failed: %s", esp_err_to_name(ret));
        bt_state = BT_STATE_DISCOVERABLE;
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t bluetooth_manager_disconnect(void)
{
    if (!bt_initialized || !device_info.is_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Disconnecting from device");
    
    esp_err_t ret = esp_a2dp_sink_disconnect(device_info.remote_addr);
    if (ret != ESP_OK) {
        DEBUG_ERROR("A2DP disconnect failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t bluetooth_manager_toggle_play(void)
{
    if (!bt_initialized || !device_info.is_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Toggling play/pause");
    
    esp_avrc_ct_cmd_t cmd = {0};
    cmd.key_code = ESP_AVRC_PT_CMD_PLAY;
    cmd.key_state = ESP_AVRC_PT_CMD_STATE_PRESSED;
    
    esp_err_t ret = esp_avrc_ct_send_passthrough_cmd(0, &cmd);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Send play command failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t bluetooth_manager_next_track(void)
{
    if (!bt_initialized || !device_info.is_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Next track");
    
    esp_avrc_ct_cmd_t cmd = {0};
    cmd.key_code = ESP_AVRC_PT_CMD_FORWARD;
    cmd.key_state = ESP_AVRC_PT_CMD_STATE_PRESSED;
    
    esp_err_t ret = esp_avrc_ct_send_passthrough_cmd(0, &cmd);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Send next track command failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t bluetooth_manager_prev_track(void)
{
    if (!bt_initialized || !device_info.is_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Previous track");
    
    esp_avrc_ct_cmd_t cmd = {0};
    cmd.key_code = ESP_AVRC_PT_CMD_BACKWARD;
    cmd.key_state = ESP_AVRC_PT_CMD_STATE_PRESSED;
    
    esp_err_t ret = esp_avrc_ct_send_passthrough_cmd(0, &cmd);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Send previous track command failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

// Event handlers
static void bt_gap_event_handler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                DEBUG_INFO("Authentication complete");
                bt_save_paired_device(param->auth_cmpl.bda);
            } else {
                DEBUG_ERROR("Authentication failed");
            }
            break;
            
        case ESP_BT_GAP_PIN_REQ_EVT:
            DEBUG_INFO("PIN request");
            esp_bt_pin_code_t pin_code;
            strcpy((char*)pin_code, DEVICE_PIN_CODE);
            esp_bt_gap_pin_reply(param->pin_req.bda, true, strlen(DEVICE_PIN_CODE), pin_code);
            break;
            
        case ESP_BT_GAP_DISC_RES_EVT:
            DEBUG_INFO("Discovery result");
            // Handle discovered devices
            break;
            
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
                DEBUG_INFO("Discovery stopped");
            } else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED) {
                DEBUG_INFO("Discovery started");
            }
            break;
            
        default:
            DEBUG_DEBUG("Unhandled GAP event: %d", event);
            break;
    }
}

static void bt_a2dp_event_handler(esp_a2dp_cb_event_t event, esp_a2dp_cb_param_t *param)
{
    switch (event) {
        case ESP_A2DP_CONNECTION_STATE_EVT:
            if (param->conn_stat.state == ESP_A2DP_CONNECTION_STATE_CONNECTED) {
                DEBUG_INFO("A2DP connected");
                device_info.is_connected = true;
                bt_state = BT_STATE_CONNECTED;
                memcpy(device_info.remote_addr, param->conn_stat.remote_bda, sizeof(esp_bd_addr_t));
                bt_send_system_event(SYSTEM_EVENT_BT_CONNECTED, NULL);
            } else if (param->conn_stat.state == ESP_A2DP_CONNECTION_STATE_DISCONNECTED) {
                DEBUG_INFO("A2DP disconnected");
                device_info.is_connected = false;
                bt_state = BT_STATE_DISCOVERABLE;
                bt_send_system_event(SYSTEM_EVENT_BT_DISCONNECTED, NULL);
            }
            break;
            
        case ESP_A2DP_AUDIO_STATE_EVT:
            if (param->audio_stat.state == ESP_A2DP_AUDIO_STATE_STARTED) {
                DEBUG_INFO("A2DP audio started");
                bt_state = BT_STATE_PLAYING;
                bt_send_system_event(SYSTEM_EVENT_AUDIO_PLAY, NULL);
            } else if (param->audio_stat.state == ESP_A2DP_AUDIO_STATE_STOPPED) {
                DEBUG_INFO("A2DP audio stopped");
                bt_state = BT_STATE_CONNECTED;
                bt_send_system_event(SYSTEM_EVENT_AUDIO_PAUSE, NULL);
            }
            break;
            
        case ESP_A2DP_AUDIO_CFG_EVT:
            DEBUG_INFO("A2DP audio config: codec=%d, sample_rate=%d, channels=%d", 
                      param->audio_cfg.mcc.type,
                      param->audio_cfg.mcc.cie.sbc[0],
                      param->audio_cfg.mcc.cie.sbc[1]);
            audio_manager_set_config(param->audio_cfg.mcc.cie.sbc[0], param->audio_cfg.mcc.cie.sbc[1]);
            break;
            
        default:
            DEBUG_DEBUG("Unhandled A2DP event: %d", event);
            break;
    }
}

static void bt_avrc_event_handler(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    switch (event) {
        case ESP_AVRC_CT_CONNECTION_STATE_EVT:
            if (param->conn_stat.connected) {
                DEBUG_INFO("AVRC connected");
                avrc_handle = param->conn_stat.remote_bda[0];
            } else {
                DEBUG_INFO("AVRC disconnected");
                avrc_handle = 0;
            }
            break;
            
        case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT:
            DEBUG_INFO("AVRC passthrough response");
            break;
            
        case ESP_AVRC_CT_METADATA_RSP_EVT:
            DEBUG_INFO("AVRC metadata response");
            break;
            
        case ESP_AVRC_CT_PLAY_STATUS_RSP_EVT:
            DEBUG_INFO("AVRC play status response");
            break;
            
        default:
            DEBUG_DEBUG("Unhandled AVRC event: %d", event);
            break;
    }
}

// Helper functions
static void bt_send_system_event(system_event_type_t type, void *data)
{
    system_event_t event = {
        .type = type,
        .data = data
    };
    
    if (system_event_queue) {
        xQueueSend(system_event_queue, &event, 0);
    }
}

static esp_err_t bt_load_paired_devices(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open("bt_devices", NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        DEBUG_INFO("No paired devices found");
        return ESP_OK;
    }
    
    size_t addr_size = sizeof(esp_bd_addr_t);
    ret = nvs_get_blob(nvs_handle, "last_device", device_info.remote_addr, &addr_size);
    if (ret == ESP_OK) {
        DEBUG_INFO("Loaded last paired device");
        if (BT_AUTO_RECONNECT) {
            // Try to reconnect to last device
            bluetooth_manager_connect(device_info.remote_addr);
        }
    }
    
    nvs_close(nvs_handle);
    return ESP_OK;
}

static esp_err_t bt_save_paired_device(esp_bd_addr_t addr)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open("bt_devices", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Failed to open NVS for writing");
        return ret;
    }
    
    ret = nvs_set_blob(nvs_handle, "last_device", addr, sizeof(esp_bd_addr_t));
    if (ret != ESP_OK) {
        DEBUG_ERROR("Failed to save paired device");
        nvs_close(nvs_handle);
        return ret;
    }
    
    ret = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    if (ret == ESP_OK) {
        DEBUG_INFO("Paired device saved");
    }
    
    return ret;
}

// Public getter functions
bt_state_t bluetooth_manager_get_state(void)
{
    return bt_state;
}

bt_device_info_t* bluetooth_manager_get_device_info(void)
{
    return &device_info;
}

const char* bluetooth_manager_get_device_name(void)
{
    return DEVICE_NAME;
}

bool bluetooth_manager_is_connected(void)
{
    return device_info.is_connected;
}
