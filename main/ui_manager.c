#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#include "config.h"
#include "ui_manager.h"
#include "bluetooth_manager.h"
#include "power_manager.h"

static const char *TAG = "UI_MANAGER";

// UI manager state
static button_info_t buttons[3] = {0};
static led_info_t status_led = {0};
static bool ui_initialized = false;
static TaskHandle_t ui_task_handle = NULL;
static TimerHandle_t button_timer = NULL;
static TimerHandle_t led_timer = NULL;

// Button definitions
#define BUTTON_PLAY_INDEX     0
#define BUTTON_VOL_UP_INDEX   1
#define BUTTON_VOL_DOWN_INDEX 2

// Function prototypes
static void ui_task(void *pvParameters);
static void ui_init_buttons(void);
static void ui_init_led(void);
static void ui_update_buttons(void);
static void ui_update_led(void);
static void ui_handle_button_press(int button_index);
static void ui_handle_button_release(int button_index);
static void ui_send_button_event(system_event_type_t event_type);
static void ui_button_timer_callback(TimerHandle_t xTimer);
static void ui_led_timer_callback(TimerHandle_t xTimer);
static void IRAM_ATTR ui_button_isr_handler(void* arg);
static uint8_t ui_calculate_breathing_brightness(uint32_t phase);

esp_err_t ui_manager_init(void)
{
    DEBUG_INFO("Initializing UI manager");
    
    // Initialize button structures
    buttons[BUTTON_PLAY_INDEX].gpio = GPIO_BUTTON_PLAY;
    buttons[BUTTON_VOL_UP_INDEX].gpio = GPIO_BUTTON_VOL_UP;
    buttons[BUTTON_VOL_DOWN_INDEX].gpio = GPIO_BUTTON_VOL_DOWN;
    
    // Initialize LED structure
    status_led.gpio = GPIO_LED_STATUS;
    status_led.state = LED_STATE_OFF;
    status_led.brightness = 255;
    
    // Initialize hardware
    ui_init_buttons();
    ui_init_led();
    
    // Create button debounce timer
    button_timer = xTimerCreate("button_timer", 
                                pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS),
                                pdTRUE, 
                                NULL, 
                                ui_button_timer_callback);
    if (button_timer == NULL) {
        DEBUG_ERROR("Failed to create button timer");
        return ESP_ERR_NO_MEM;
    }
    
    // Create LED update timer
    led_timer = xTimerCreate("led_timer", 
                             pdMS_TO_TICKS(50), // 50ms for smooth LED effects
                             pdTRUE, 
                             NULL, 
                             ui_led_timer_callback);
    if (led_timer == NULL) {
        DEBUG_ERROR("Failed to create LED timer");
        xTimerDelete(button_timer, 0);
        return ESP_ERR_NO_MEM;
    }
    
    // Create UI task
    if (xTaskCreate(ui_task, "ui_task", 2048, NULL, 5, &ui_task_handle) != pdPASS) {
        DEBUG_ERROR("Failed to create UI task");
        xTimerDelete(button_timer, 0);
        xTimerDelete(led_timer, 0);
        return ESP_ERR_NO_MEM;
    }
    
    // Start timers
    xTimerStart(button_timer, 0);
    xTimerStart(led_timer, 0);
    
    ui_initialized = true;
    
    DEBUG_INFO("UI manager initialized successfully");
    return ESP_OK;
}

esp_err_t ui_manager_deinit(void)
{
    if (!ui_initialized) {
        return ESP_OK;
    }
    
    DEBUG_INFO("Deinitializing UI manager");
    
    // Stop and delete timers
    if (button_timer) {
        xTimerStop(button_timer, 0);
        xTimerDelete(button_timer, 0);
        button_timer = NULL;
    }
    
    if (led_timer) {
        xTimerStop(led_timer, 0);
        xTimerDelete(led_timer, 0);
        led_timer = NULL;
    }
    
    // Delete UI task
    if (ui_task_handle) {
        vTaskDelete(ui_task_handle);
        ui_task_handle = NULL;
    }
    
    // Turn off LED
    gpio_set_level(status_led.gpio, 0);
    
    // Disable button interrupts
    for (int i = 0; i < 3; i++) {
        gpio_isr_handler_remove(buttons[i].gpio);
    }
    
    ui_initialized = false;
    
    DEBUG_INFO("UI manager deinitialized");
    return ESP_OK;
}

esp_err_t ui_manager_update(void)
{
    if (!ui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Update is handled by timers and task
    return ESP_OK;
}

esp_err_t ui_manager_set_status_led(led_state_t state)
{
    if (!ui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_INFO("Setting LED state to %d", state);
    
    status_led.state = state;
    status_led.pattern_phase = 0;
    
    return ESP_OK;
}

esp_err_t ui_manager_set_led_brightness(uint8_t brightness)
{
    if (!ui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    status_led.brightness = brightness;
    return ESP_OK;
}

// Initialize buttons
static void ui_init_buttons(void)
{
    // Configure button GPIOs
    gpio_config_t button_config = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    
    for (int i = 0; i < 3; i++) {
        button_config.pin_bit_mask = (1ULL << buttons[i].gpio);
        gpio_config(&button_config);
        
        // Install ISR handler
        gpio_isr_handler_add(buttons[i].gpio, ui_button_isr_handler, (void*)i);
        
        // Initialize button state
        buttons[i].state = BUTTON_STATE_IDLE;
        buttons[i].is_pressed = false;
        buttons[i].press_time = 0;
        buttons[i].release_time = 0;
        buttons[i].click_count = 0;
    }
    
    // Install GPIO ISR service
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
}

// Initialize LED
static void ui_init_led(void)
{
    // Configure LED GPIO
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << status_led.gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_config);
    
    // Initialize LED PWM for brightness control
    ledc_timer_config_t timer_config = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);
    
    ledc_channel_config_t channel_config = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = status_led.gpio,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);
    
    // Turn off LED initially
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

// UI task
static void ui_task(void *pvParameters)
{
    DEBUG_INFO("UI task started");
    
    while (1) {
        // Task is event-driven by timers
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Button timer callback
static void ui_button_timer_callback(TimerHandle_t xTimer)
{
    ui_update_buttons();
}

// LED timer callback
static void ui_led_timer_callback(TimerHandle_t xTimer)
{
    ui_update_led();
}

// Update buttons
static void ui_update_buttons(void)
{
    uint32_t current_time = xTaskGetTickCount();
    
    for (int i = 0; i < 3; i++) {
        bool current_pressed = !gpio_get_level(buttons[i].gpio); // Active low
        
        // Handle button press
        if (current_pressed && !buttons[i].is_pressed) {
            buttons[i].is_pressed = true;
            buttons[i].press_time = current_time;
            buttons[i].state = BUTTON_STATE_PRESSED;
            ui_handle_button_press(i);
        }
        // Handle button release
        else if (!current_pressed && buttons[i].is_pressed) {
            buttons[i].is_pressed = false;
            buttons[i].release_time = current_time;
            
            uint32_t press_duration = current_time - buttons[i].press_time;
            
            // Check for long press
            if (press_duration > pdMS_TO_TICKS(BUTTON_LONG_PRESS_MS)) {
                buttons[i].state = BUTTON_STATE_LONG_PRESSED;
            } else {
                buttons[i].click_count++;
                
                // Check for double click
                if (buttons[i].click_count >= 2) {
                    buttons[i].state = BUTTON_STATE_DOUBLE_CLICKED;
                    buttons[i].click_count = 0;
                }
            }
            
            ui_handle_button_release(i);
        }
        
        // Reset click count after timeout
        if (buttons[i].click_count > 0 && 
            (current_time - buttons[i].release_time) > pdMS_TO_TICKS(BUTTON_DOUBLE_CLICK_MS)) {
            buttons[i].click_count = 0;
            buttons[i].state = BUTTON_STATE_IDLE;
        }
    }
}

// Update LED
static void ui_update_led(void)
{
    uint8_t duty = 0;
    
    switch (status_led.state) {
        case LED_STATE_OFF:
            duty = 0;
            break;
            
        case LED_STATE_ON:
            duty = status_led.brightness;
            break;
            
        case LED_STATE_SLOW_BLINK:
            // 1 second cycle
            duty = ((status_led.pattern_phase % 20) < 10) ? status_led.brightness : 0;
            break;
            
        case LED_STATE_FAST_BLINK:
            // 0.5 second cycle
            duty = ((status_led.pattern_phase % 10) < 5) ? status_led.brightness : 0;
            break;
            
        case LED_STATE_BREATHING:
            duty = ui_calculate_breathing_brightness(status_led.pattern_phase);
            break;
    }
    
    // Set LED duty cycle
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    status_led.pattern_phase++;
    if (status_led.pattern_phase >= 1000) {
        status_led.pattern_phase = 0;
    }
}

// Handle button press
static void ui_handle_button_press(int button_index)
{
    DEBUG_DEBUG("Button %d pressed", button_index);
    
    // Signal activity to power manager
    power_manager_activity_detected();
    
    // Send appropriate event based on button
    switch (button_index) {
        case BUTTON_PLAY_INDEX:
            ui_send_button_event(SYSTEM_EVENT_BUTTON_PLAY);
            break;
            
        case BUTTON_VOL_UP_INDEX:
            ui_send_button_event(SYSTEM_EVENT_BUTTON_VOL_UP);
            break;
            
        case BUTTON_VOL_DOWN_INDEX:
            ui_send_button_event(SYSTEM_EVENT_BUTTON_VOL_DOWN);
            break;
    }
}

// Handle button release
static void ui_handle_button_release(int button_index)
{
    DEBUG_DEBUG("Button %d released (state: %d)", button_index, buttons[button_index].state);
    
    // Handle different button states
    switch (buttons[button_index].state) {
        case BUTTON_STATE_LONG_PRESSED:
            // Handle long press actions
            if (button_index == BUTTON_PLAY_INDEX) {
                DEBUG_INFO("Long press detected - power toggle");
                // Could implement power toggle here
            }
            break;
            
        case BUTTON_STATE_DOUBLE_CLICKED:
            // Handle double click actions
            if (button_index == BUTTON_PLAY_INDEX) {
                DEBUG_INFO("Double click detected - next track");
                // Could implement next track here
            }
            break;
            
        default:
            // Single click already handled in press
            break;
    }
    
    buttons[button_index].state = BUTTON_STATE_IDLE;
}

// Send button event
static void ui_send_button_event(system_event_type_t event_type)
{
    system_event_t event = {
        .type = event_type,
        .data = NULL
    };
    
    if (system_event_queue) {
        xQueueSend(system_event_queue, &event, 0);
    }
}

// Button ISR handler
static void IRAM_ATTR ui_button_isr_handler(void* arg)
{
    // ISR should be minimal - actual processing done in timer callback
    // Just signal that button state changed
}

// Calculate breathing brightness
static uint8_t ui_calculate_breathing_brightness(uint32_t phase)
{
    // 2 second breathing cycle
    float cycle_pos = (float)(phase % 40) / 40.0f;
    float brightness = (sin(cycle_pos * 2.0f * M_PI) + 1.0f) / 2.0f;
    
    return (uint8_t)(brightness * status_led.brightness);
}

// Getter functions
button_state_t ui_manager_get_button_state(gpio_num_t gpio)
{
    for (int i = 0; i < 3; i++) {
        if (buttons[i].gpio == gpio) {
            return buttons[i].state;
        }
    }
    return BUTTON_STATE_IDLE;
}

led_state_t ui_manager_get_led_state(void)
{
    return status_led.state;
}
