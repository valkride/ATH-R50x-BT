#ifndef CONFIG_H
#define CONFIG_H

#include "sdkconfig.h"

// Device Information
#define DEVICE_NAME             CONFIG_BT_HEADSET_DEVICE_NAME
#define DEVICE_PIN_CODE         CONFIG_BT_HEADSET_PIN_CODE
#define DEVICE_VERSION          "1.0.0"

// Audio Configuration
#define AUDIO_SAMPLE_RATE       CONFIG_AUDIO_SAMPLE_RATE
#define AUDIO_BITS_PER_SAMPLE   CONFIG_AUDIO_BITS_PER_SAMPLE
#define AUDIO_CHANNELS          CONFIG_AUDIO_CHANNELS
#define AUDIO_BUFFER_SIZE       CONFIG_AUDIO_BUFFER_SIZE

// GPIO Pin Definitions
#define GPIO_I2S_BCK            CONFIG_GPIO_I2S_BCK
#define GPIO_I2S_WS             CONFIG_GPIO_I2S_WS
#define GPIO_I2S_DATA           CONFIG_GPIO_I2S_DATA
#define GPIO_BUTTON_PLAY        CONFIG_GPIO_BUTTON_PLAY
#define GPIO_BUTTON_VOL_UP      CONFIG_GPIO_BUTTON_VOL_UP
#define GPIO_BUTTON_VOL_DOWN    CONFIG_GPIO_BUTTON_VOL_DOWN
#define GPIO_LED_STATUS         CONFIG_GPIO_LED_STATUS
#define GPIO_BATTERY_ADC        CONFIG_GPIO_BATTERY_ADC

// Power Management
#define POWER_SLEEP_TIMEOUT_MS          (CONFIG_POWER_SLEEP_TIMEOUT * 1000)
#define POWER_LOW_BATTERY_THRESHOLD_MV  CONFIG_POWER_LOW_BATTERY_THRESHOLD
#define POWER_SHUTDOWN_THRESHOLD_MV     CONFIG_POWER_SHUTDOWN_THRESHOLD
#define POWER_BATTERY_CHECK_INTERVAL_MS (CONFIG_POWER_BATTERY_CHECK_INTERVAL * 1000)

// Bluetooth Configuration
#define BT_AUTO_RECONNECT       CONFIG_BT_HEADSET_AUTO_RECONNECT
#define BT_DISCOVERY_TIME       30000  // 30 seconds
#define BT_RECONNECT_ATTEMPTS   5

// Audio Processing
#define AUDIO_VOLUME_STEPS      16
#define AUDIO_VOLUME_DEFAULT    8
#define AUDIO_FADE_DURATION_MS  500

// LED Patterns
#define LED_PATTERN_OFF         0
#define LED_PATTERN_SOLID       1
#define LED_PATTERN_SLOW_BLINK  2
#define LED_PATTERN_FAST_BLINK  3
#define LED_PATTERN_BREATHING   4

// Button Timing
#define BUTTON_DEBOUNCE_MS      50
#define BUTTON_LONG_PRESS_MS    1000
#define BUTTON_DOUBLE_CLICK_MS  300

// System Settings
#define SYSTEM_TASK_STACK_SIZE  4096
#define SYSTEM_TASK_PRIORITY    5
#define SYSTEM_QUEUE_SIZE       10

// Debug Settings
#define DEBUG_ENABLED           1
#define DEBUG_LEVEL             3  // 0=None, 1=Error, 2=Warning, 3=Info, 4=Debug

#if DEBUG_ENABLED
#define DEBUG_PRINT(level, fmt, ...) \
    do { \
        if (level <= DEBUG_LEVEL) { \
            printf("[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)
#else
#define DEBUG_PRINT(level, fmt, ...)
#endif

#define DEBUG_ERROR(fmt, ...)   DEBUG_PRINT(1, "ERROR: " fmt, ##__VA_ARGS__)
#define DEBUG_WARN(fmt, ...)    DEBUG_PRINT(2, "WARN: " fmt, ##__VA_ARGS__)
#define DEBUG_INFO(fmt, ...)    DEBUG_PRINT(3, "INFO: " fmt, ##__VA_ARGS__)
#define DEBUG_DEBUG(fmt, ...)   DEBUG_PRINT(4, "DEBUG: " fmt, ##__VA_ARGS__)

#endif // CONFIG_H
