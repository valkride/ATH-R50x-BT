#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "config.h"
#include "bluetooth_manager.h"
#include "audio_manager.h"
#include "power_manager.h"
#include "ui_manager.h"

static const char *TAG = "MAIN";

// Event group for system coordination
EventGroupHandle_t system_event_group;

// System event bits
#define SYSTEM_BT_READY_BIT     BIT0
#define SYSTEM_AUDIO_READY_BIT  BIT1
#define SYSTEM_POWER_READY_BIT  BIT2
#define SYSTEM_UI_READY_BIT     BIT3
#define SYSTEM_ALL_READY_BITS   (SYSTEM_BT_READY_BIT | SYSTEM_AUDIO_READY_BIT | SYSTEM_POWER_READY_BIT | SYSTEM_UI_READY_BIT)

// System state
typedef enum {
    SYSTEM_STATE_INIT,
    SYSTEM_STATE_READY,
    SYSTEM_STATE_CONNECTED,
    SYSTEM_STATE_PLAYING,
    SYSTEM_STATE_SLEEP,
    SYSTEM_STATE_ERROR
} system_state_t;

static system_state_t system_state = SYSTEM_STATE_INIT;

// Function prototypes
static void system_init(void);
static void system_main_task(void *pvParameters);
static void system_event_handler(system_event_t event);
static void system_state_machine(system_event_t event);

void app_main(void)
{
    DEBUG_INFO("Starting ESP32-C3 Bluetooth Headset Module v%s", DEVICE_VERSION);
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize system
    system_init();
    
    // Create main system task
    xTaskCreate(system_main_task, "system_main", SYSTEM_TASK_STACK_SIZE, NULL, SYSTEM_TASK_PRIORITY, NULL);
    
    DEBUG_INFO("System initialization complete");
}

static void system_init(void)
{
    // Create event group for system coordination
    system_event_group = xEventGroupCreate();
    if (system_event_group == NULL) {
        DEBUG_ERROR("Failed to create system event group");
        esp_restart();
    }
    
    // Initialize all subsystems
    DEBUG_INFO("Initializing subsystems...");
    
    // Initialize power management first
    if (power_manager_init() != ESP_OK) {
        DEBUG_ERROR("Power manager initialization failed");
        esp_restart();
    }
    xEventGroupSetBits(system_event_group, SYSTEM_POWER_READY_BIT);
    
    // Initialize audio system
    if (audio_manager_init() != ESP_OK) {
        DEBUG_ERROR("Audio manager initialization failed");
        esp_restart();
    }
    xEventGroupSetBits(system_event_group, SYSTEM_AUDIO_READY_BIT);
    
    // Initialize UI system
    if (ui_manager_init() != ESP_OK) {
        DEBUG_ERROR("UI manager initialization failed");
        esp_restart();
    }
    xEventGroupSetBits(system_event_group, SYSTEM_UI_READY_BIT);
    
    // Initialize Bluetooth last
    if (bluetooth_manager_init() != ESP_OK) {
        DEBUG_ERROR("Bluetooth manager initialization failed");
        esp_restart();
    }
    xEventGroupSetBits(system_event_group, SYSTEM_BT_READY_BIT);
    
    DEBUG_INFO("All subsystems initialized");
}

static void system_main_task(void *pvParameters)
{
    system_event_t event;
    
    // Wait for all subsystems to be ready
    xEventGroupWaitBits(system_event_group, SYSTEM_ALL_READY_BITS, pdFALSE, pdTRUE, portMAX_DELAY);
    
    system_state = SYSTEM_STATE_READY;
    ui_manager_set_status_led(LED_PATTERN_SLOW_BLINK);
    
    DEBUG_INFO("System ready - entering main loop");
    
    while (1) {
        // Check for system events
        if (xQueueReceive(system_event_queue, &event, pdMS_TO_TICKS(100)) == pdTRUE) {
            system_event_handler(event);
        }
        
        // Periodic system maintenance
        power_manager_update();
        ui_manager_update();
        
        // Check system health
        if (power_manager_get_battery_voltage() < POWER_SHUTDOWN_THRESHOLD_MV) {
            DEBUG_WARN("Battery critically low - shutting down");
            system_state = SYSTEM_STATE_ERROR;
            power_manager_shutdown();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void system_event_handler(system_event_t event)
{
    DEBUG_DEBUG("System event: %d", event.type);
    
    switch (event.type) {
        case SYSTEM_EVENT_BT_CONNECTED:
            DEBUG_INFO("Bluetooth connected");
            system_state = SYSTEM_STATE_CONNECTED;
            ui_manager_set_status_led(LED_PATTERN_SOLID);
            break;
            
        case SYSTEM_EVENT_BT_DISCONNECTED:
            DEBUG_INFO("Bluetooth disconnected");
            system_state = SYSTEM_STATE_READY;
            ui_manager_set_status_led(LED_PATTERN_SLOW_BLINK);
            audio_manager_stop();
            break;
            
        case SYSTEM_EVENT_AUDIO_PLAY:
            DEBUG_INFO("Audio playback started");
            system_state = SYSTEM_STATE_PLAYING;
            break;
            
        case SYSTEM_EVENT_AUDIO_PAUSE:
            DEBUG_INFO("Audio playback paused");
            system_state = SYSTEM_STATE_CONNECTED;
            break;
            
        case SYSTEM_EVENT_BUTTON_PLAY:
            DEBUG_INFO("Play button pressed");
            if (system_state == SYSTEM_STATE_CONNECTED || system_state == SYSTEM_STATE_PLAYING) {
                bluetooth_manager_toggle_play();
            }
            break;
            
        case SYSTEM_EVENT_BUTTON_VOL_UP:
            DEBUG_INFO("Volume up button pressed");
            if (system_state == SYSTEM_STATE_CONNECTED || system_state == SYSTEM_STATE_PLAYING) {
                audio_manager_volume_up();
            }
            break;
            
        case SYSTEM_EVENT_BUTTON_VOL_DOWN:
            DEBUG_INFO("Volume down button pressed");
            if (system_state == SYSTEM_STATE_CONNECTED || system_state == SYSTEM_STATE_PLAYING) {
                audio_manager_volume_down();
            }
            break;
            
        case SYSTEM_EVENT_POWER_LOW:
            DEBUG_WARN("Low battery warning");
            ui_manager_set_status_led(LED_PATTERN_FAST_BLINK);
            break;
            
        case SYSTEM_EVENT_POWER_SLEEP:
            DEBUG_INFO("Entering sleep mode");
            system_state = SYSTEM_STATE_SLEEP;
            power_manager_enter_sleep();
            break;
            
        case SYSTEM_EVENT_POWER_WAKE:
            DEBUG_INFO("Waking from sleep");
            system_state = SYSTEM_STATE_READY;
            ui_manager_set_status_led(LED_PATTERN_SLOW_BLINK);
            break;
            
        default:
            DEBUG_WARN("Unknown system event: %d", event.type);
            break;
    }
}

// System information functions
const char* system_get_device_name(void)
{
    return DEVICE_NAME;
}

const char* system_get_version(void)
{
    return DEVICE_VERSION;
}

system_state_t system_get_state(void)
{
    return system_state;
}

// Error handling
void system_error_handler(const char* error_msg)
{
    DEBUG_ERROR("System error: %s", error_msg);
    system_state = SYSTEM_STATE_ERROR;
    ui_manager_set_status_led(LED_PATTERN_FAST_BLINK);
    
    // Try to recover or restart
    vTaskDelay(pdMS_TO_TICKS(5000));
    esp_restart();
}
