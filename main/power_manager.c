#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_pm.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_adc_cal.h"

#include "config.h"
#include "power_manager.h"
#include "bluetooth_manager.h"

static const char *TAG = "POWER_MANAGER";

// Power manager state
static power_state_t power_state = POWER_STATE_ACTIVE;
static battery_info_t battery_info = {0};
static bool power_initialized = false;
static uint32_t activity_timeout_ms = POWER_SLEEP_TIMEOUT_MS;
static uint32_t last_activity_time = 0;

// ADC calibration
static esp_adc_cal_characteristics_t *adc_chars = NULL;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

// Timers
static TimerHandle_t battery_check_timer = NULL;
static TimerHandle_t sleep_timer = NULL;

// Function prototypes
static void power_init_adc(void);
static void power_battery_check_callback(TimerHandle_t xTimer);
static void power_sleep_timer_callback(TimerHandle_t xTimer);
static uint16_t power_read_battery_voltage(void);
static uint8_t power_calculate_battery_percentage(uint16_t voltage_mv);
static void power_update_battery_info(void);
static esp_err_t power_configure_wake_sources(void);
static void power_send_low_battery_event(void);

esp_err_t power_manager_init(void)
{
    DEBUG_INFO("Initializing power manager");
    
    // Initialize ADC for battery monitoring
    power_init_adc();
    
    // Configure power management
    esp_pm_config_esp32c3_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret != ESP_OK) {
        DEBUG_WARN("Power management config failed: %s", esp_err_to_name(ret));
    }
    
    // Create battery check timer
    battery_check_timer = xTimerCreate("battery_check", 
                                       pdMS_TO_TICKS(POWER_BATTERY_CHECK_INTERVAL_MS),
                                       pdTRUE, 
                                       NULL, 
                                       power_battery_check_callback);
    if (battery_check_timer == NULL) {
        DEBUG_ERROR("Failed to create battery check timer");
        return ESP_ERR_NO_MEM;
    }
    
    // Create sleep timer
    sleep_timer = xTimerCreate("sleep_timer", 
                               pdMS_TO_TICKS(activity_timeout_ms),
                               pdFALSE, 
                               NULL, 
                               power_sleep_timer_callback);
    if (sleep_timer == NULL) {
        DEBUG_ERROR("Failed to create sleep timer");
        xTimerDelete(battery_check_timer, 0);
        return ESP_ERR_NO_MEM;
    }
    
    // Start battery monitoring
    xTimerStart(battery_check_timer, 0);
    
    // Start sleep timer
    xTimerStart(sleep_timer, 0);
    
    // Configure wake-up sources
    power_configure_wake_sources();
    
    // Initial battery reading
    power_update_battery_info();
    
    power_initialized = true;
    power_state = POWER_STATE_ACTIVE;
    last_activity_time = xTaskGetTickCount();
    
    DEBUG_INFO("Power manager initialized successfully");
    DEBUG_INFO("Battery voltage: %d mV (%d%%)", battery_info.voltage_mv, battery_info.percentage);
    
    return ESP_OK;
}

esp_err_t power_manager_deinit(void)
{
    if (!power_initialized) {
        return ESP_OK;
    }
    
    DEBUG_INFO("Deinitializing power manager");
    
    // Stop and delete timers
    if (battery_check_timer) {
        xTimerStop(battery_check_timer, 0);
        xTimerDelete(battery_check_timer, 0);
        battery_check_timer = NULL;
    }
    
    if (sleep_timer) {
        xTimerStop(sleep_timer, 0);
        xTimerDelete(sleep_timer, 0);
        sleep_timer = NULL;
    }
    
    // Deinitialize ADC
    if (adc_chars) {
        free(adc_chars);
        adc_chars = NULL;
    }
    
    power_initialized = false;
    power_state = POWER_STATE_ACTIVE;
    
    DEBUG_INFO("Power manager deinitialized");
    return ESP_OK;
}

esp_err_t power_manager_update(void)
{
    if (!power_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Check if it's time to sleep
    uint32_t current_time = xTaskGetTickCount();
    if (current_time - last_activity_time > pdMS_TO_TICKS(activity_timeout_ms)) {
        if (power_state == POWER_STATE_ACTIVE) {
            power_state = POWER_STATE_IDLE;
            DEBUG_INFO("System idle - ready for sleep");
        }
    }
    
    return ESP_OK;
}

esp_err_t power_manager_enter_sleep(void)
{
    if (!power_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Entering sleep mode");
    
    power_state = POWER_STATE_SLEEP;
    
    // Configure wake-up sources
    power_configure_wake_sources();
    
    // Enter light sleep
    esp_err_t ret = esp_light_sleep_start();
    if (ret != ESP_OK) {
        DEBUG_ERROR("Light sleep failed: %s", esp_err_to_name(ret));
        power_state = POWER_STATE_ACTIVE;
        return ret;
    }
    
    // Wake up
    DEBUG_INFO("Waking up from sleep");
    power_state = POWER_STATE_ACTIVE;
    last_activity_time = xTaskGetTickCount();
    
    // Send wake event
    system_event_t wake_event = {
        .type = SYSTEM_EVENT_POWER_WAKE,
        .data = NULL
    };
    xQueueSend(system_event_queue, &wake_event, 0);
    
    return ESP_OK;
}

esp_err_t power_manager_wake_up(void)
{
    if (!power_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Wake up requested");
    
    power_state = POWER_STATE_ACTIVE;
    power_manager_activity_detected();
    
    return ESP_OK;
}

esp_err_t power_manager_shutdown(void)
{
    DEBUG_INFO("Shutting down system");
    
    power_state = POWER_STATE_SHUTDOWN;
    
    // Save any important data
    // Disconnect Bluetooth
    bluetooth_manager_disconnect();
    
    // Enter deep sleep (effectively shutdown)
    esp_deep_sleep_start();
    
    return ESP_OK;
}

esp_err_t power_manager_set_activity_timeout(uint32_t timeout_ms)
{
    if (!power_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    activity_timeout_ms = timeout_ms;
    
    // Reset sleep timer with new timeout
    if (sleep_timer) {
        xTimerStop(sleep_timer, 0);
        xTimerChangePeriod(sleep_timer, pdMS_TO_TICKS(timeout_ms), 0);
        xTimerStart(sleep_timer, 0);
    }
    
    DEBUG_INFO("Activity timeout set to %d ms", timeout_ms);
    return ESP_OK;
}

void power_manager_activity_detected(void)
{
    if (!power_initialized) {
        return;
    }
    
    last_activity_time = xTaskGetTickCount();
    
    // Reset sleep timer
    if (sleep_timer) {
        xTimerReset(sleep_timer, 0);
    }
    
    // Wake up if sleeping
    if (power_state == POWER_STATE_SLEEP || power_state == POWER_STATE_IDLE) {
        power_state = POWER_STATE_ACTIVE;
    }
}

// Initialize ADC for battery monitoring
static void power_init_adc(void)
{
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(GPIO_BATTERY_ADC, atten);
    
    // Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, 1100, adc_chars);
}

// Battery check timer callback
static void power_battery_check_callback(TimerHandle_t xTimer)
{
    power_update_battery_info();
    
    // Check for low battery
    if (battery_info.voltage_mv < POWER_LOW_BATTERY_THRESHOLD_MV && !battery_info.is_low) {
        battery_info.is_low = true;
        power_send_low_battery_event();
    }
    
    // Check for critical battery
    if (battery_info.voltage_mv < POWER_SHUTDOWN_THRESHOLD_MV && !battery_info.is_critical) {
        battery_info.is_critical = true;
        DEBUG_ERROR("Critical battery level - shutting down");
        power_manager_shutdown();
    }
}

// Sleep timer callback
static void power_sleep_timer_callback(TimerHandle_t xTimer)
{
    if (power_state == POWER_STATE_ACTIVE) {
        DEBUG_INFO("Sleep timer expired - entering sleep");
        
        system_event_t sleep_event = {
            .type = SYSTEM_EVENT_POWER_SLEEP,
            .data = NULL
        };
        xQueueSend(system_event_queue, &sleep_event, 0);
    }
}

// Read battery voltage
static uint16_t power_read_battery_voltage(void)
{
    uint32_t adc_reading = 0;
    
    // Take multiple readings and average
    for (int i = 0; i < 64; i++) {
        adc_reading += adc1_get_raw(GPIO_BATTERY_ADC);
    }
    adc_reading /= 64;
    
    // Convert to voltage
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    
    // Account for voltage divider (if used)
    // Assuming 2:1 voltage divider for Li-ion battery
    voltage *= 2;
    
    return (uint16_t)voltage;
}

// Calculate battery percentage
static uint8_t power_calculate_battery_percentage(uint16_t voltage_mv)
{
    // Li-ion battery voltage curve (approximate)
    const uint16_t battery_curve[] = {
        3200, 3300, 3400, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4200
    };
    const uint8_t curve_size = sizeof(battery_curve) / sizeof(battery_curve[0]);
    
    if (voltage_mv <= battery_curve[0]) {
        return 0;
    }
    
    if (voltage_mv >= battery_curve[curve_size - 1]) {
        return 100;
    }
    
    for (int i = 0; i < curve_size - 1; i++) {
        if (voltage_mv >= battery_curve[i] && voltage_mv < battery_curve[i + 1]) {
            uint16_t range = battery_curve[i + 1] - battery_curve[i];
            uint16_t offset = voltage_mv - battery_curve[i];
            uint8_t base_percentage = (i * 100) / (curve_size - 1);
            uint8_t percentage_range = 100 / (curve_size - 1);
            
            return base_percentage + ((offset * percentage_range) / range);
        }
    }
    
    return 50; // Default fallback
}

// Update battery information
static void power_update_battery_info(void)
{
    battery_info.voltage_mv = power_read_battery_voltage();
    battery_info.percentage = power_calculate_battery_percentage(battery_info.voltage_mv);
    
    // Check charging status (if charging circuit is connected)
    // This would require additional hardware and GPIO
    battery_info.is_charging = false;
    
    // Update low battery status
    if (battery_info.voltage_mv >= POWER_LOW_BATTERY_THRESHOLD_MV) {
        battery_info.is_low = false;
    }
    
    // Update critical battery status
    if (battery_info.voltage_mv >= POWER_SHUTDOWN_THRESHOLD_MV) {
        battery_info.is_critical = false;
    }
}

// Configure wake-up sources
static esp_err_t power_configure_wake_sources(void)
{
    // Configure GPIO wake-up (button press)
    esp_err_t ret = esp_sleep_enable_ext0_wakeup(GPIO_BUTTON_PLAY, 0);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Failed to configure GPIO wakeup: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure timer wake-up (periodic wake for battery check)
    ret = esp_sleep_enable_timer_wakeup(POWER_BATTERY_CHECK_INTERVAL_MS * 1000);
    if (ret != ESP_OK) {
        DEBUG_ERROR("Failed to configure timer wakeup: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

// Send low battery event
static void power_send_low_battery_event(void)
{
    system_event_t low_battery_event = {
        .type = SYSTEM_EVENT_POWER_LOW,
        .data = NULL
    };
    
    if (system_event_queue) {
        xQueueSend(system_event_queue, &low_battery_event, 0);
    }
}

// Getter functions
uint16_t power_manager_get_battery_voltage(void)
{
    return battery_info.voltage_mv;
}

uint8_t power_manager_get_battery_percentage(void)
{
    return battery_info.percentage;
}

battery_info_t* power_manager_get_battery_info(void)
{
    return &battery_info;
}

power_state_t power_manager_get_state(void)
{
    return power_state;
}

bool power_manager_is_charging(void)
{
    return battery_info.is_charging;
}
