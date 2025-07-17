#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "esp_err.h"
#include "esp_sleep.h"

// Power manager types
typedef enum {
    POWER_STATE_ACTIVE,
    POWER_STATE_IDLE,
    POWER_STATE_SLEEP,
    POWER_STATE_DEEP_SLEEP,
    POWER_STATE_SHUTDOWN
} power_state_t;

typedef struct {
    uint16_t voltage_mv;
    uint8_t percentage;
    bool is_charging;
    bool is_low;
    bool is_critical;
} battery_info_t;

// Function prototypes
esp_err_t power_manager_init(void);
esp_err_t power_manager_deinit(void);
esp_err_t power_manager_update(void);
esp_err_t power_manager_enter_sleep(void);
esp_err_t power_manager_wake_up(void);
esp_err_t power_manager_shutdown(void);
esp_err_t power_manager_set_activity_timeout(uint32_t timeout_ms);
uint16_t power_manager_get_battery_voltage(void);
uint8_t power_manager_get_battery_percentage(void);
battery_info_t* power_manager_get_battery_info(void);
power_state_t power_manager_get_state(void);
bool power_manager_is_charging(void);
void power_manager_activity_detected(void);

#endif // POWER_MANAGER_H
