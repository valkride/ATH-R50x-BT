#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "esp_err.h"
#include "driver/gpio.h"

// UI manager types
typedef enum {
    BUTTON_STATE_IDLE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_LONG_PRESSED,
    BUTTON_STATE_DOUBLE_CLICKED
} button_state_t;

typedef enum {
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_SLOW_BLINK,
    LED_STATE_FAST_BLINK,
    LED_STATE_BREATHING
} led_state_t;

typedef struct {
    gpio_num_t gpio;
    button_state_t state;
    uint32_t press_time;
    uint32_t release_time;
    uint8_t click_count;
    bool is_pressed;
} button_info_t;

typedef struct {
    gpio_num_t gpio;
    led_state_t state;
    uint8_t brightness;
    uint32_t pattern_phase;
    bool is_on;
} led_info_t;

// Function prototypes
esp_err_t ui_manager_init(void);
esp_err_t ui_manager_deinit(void);
esp_err_t ui_manager_update(void);
esp_err_t ui_manager_set_status_led(led_state_t state);
esp_err_t ui_manager_set_led_brightness(uint8_t brightness);
button_state_t ui_manager_get_button_state(gpio_num_t gpio);
led_state_t ui_manager_get_led_state(void);

#endif // UI_MANAGER_H
