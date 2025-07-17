/**
 * Configuration File for ESP32-C3 SuperMini Bluetooth Headset
 * 
 * This file contains all configuration parameters, pin definitions,
 * and system constants for the headset firmware.
 * 
 * Author: ESP32-C3 Audio Team
 * Date: July 2025
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ====================================================================================
// FIRMWARE VERSION AND INFO
// ====================================================================================

#define FIRMWARE_VERSION        "1.0.0"
#define FIRMWARE_BUILD_DATE     __DATE__ " " __TIME__
#define DEVICE_NAME             "ESP32-C3-SuperMini-Headset"
#define MANUFACTURER            "ESP32-C3 Audio Team"

// ====================================================================================
// HARDWARE PIN DEFINITIONS
// ====================================================================================

// I2C OLED Display
#define PIN_OLED_SDA            21  // GPIO21 - SDA
#define PIN_OLED_SCL            22  // GPIO22 - SCL

// ADC and Status Monitoring
#define PIN_BAT_ADC             35  // GPIO35 - Battery voltage divider (ADC1_CH7)
#define PIN_STAT                34  // GPIO34 - TP4056 STAT line from right cup
#define PIN_TEMP_SENSOR         20  // GPIO20 - Optional temperature sensor

// Power Control
#define PIN_EN_AUDIO            25  // GPIO25 - Enable QCC5124 + TPA6120A2 LDO
#define PIN_EN_MIC              26  // GPIO26 - Enable mic module in right cup (P-MOSFET gate)
#define PIN_EN_USB              21  // GPIO21 - USB power enable (optional)

// User Interface - Buttons (active low with internal pullup)
#define PIN_BTN_PWR             27  // GPIO27 - Power button
#define PIN_BTN_VOL_UP          14  // GPIO14 - Volume up button
#define PIN_BTN_VOL_DN          12  // GPIO12 - Volume down button
#define PIN_BTN_MUTE            13  // GPIO13 - Mute button

// QCC5124 A2DP Codec Control
#define PIN_QCC_TX              10  // GPIO10 - UART TX to QCC5124
#define PIN_QCC_RX              9   // GPIO9 - UART RX from QCC5124
#define PIN_QCC_RST             2   // GPIO2 - QCC5124 Reset

// I2S Microphone Input (for VAD and audio processing)
#define PIN_I2S_WS              18  // GPIO18 - I2S Word Select
#define PIN_I2S_SCK             19  // GPIO19 - I2S Serial Clock
#define PIN_I2S_SD              23  // GPIO23 - I2S Serial Data

// Status LEDs (optional)
#define PIN_LED_STATUS          8   // GPIO8 - Status LED
#define PIN_LED_POWER           1   // GPIO1 - Power LED

// ====================================================================================
// DISPLAY CONFIGURATION
// ====================================================================================

#define SCREEN_WIDTH            128
#define SCREEN_HEIGHT           32
#define OLED_RESET              -1
#define OLED_I2C_ADDR           0x3C
#define DISPLAY_TIMEOUT_MS      30000   // Turn off display after 30 seconds
#define DISPLAY_UPDATE_RATE_MS  100     // Update display every 100ms

// ====================================================================================
// BATTERY MONITORING CONFIGURATION
// ====================================================================================

#define BAT_VOLTAGE_DIVIDER     2.0f    // 2:1 voltage divider
#define BAT_FULL_VOLTAGE        4.2f    // Li-ion full voltage
#define BAT_EMPTY_VOLTAGE       3.0f    // Li-ion empty voltage
#define BAT_CRITICAL_VOLTAGE    3.2f    // Critical voltage for shutdown
#define BAT_LOW_VOLTAGE         3.4f    // Low battery warning
#define BAT_SAMPLES             64      // ADC averaging samples
#define BAT_CHECK_INTERVAL_MS   5000    // Check battery every 5 seconds

// ====================================================================================
// BUTTON CONFIGURATION
// ====================================================================================

#define BUTTON_DEBOUNCE_MS      50      // Debounce time
#define BUTTON_LONG_PRESS_MS    1000    // Long press threshold
#define BUTTON_DOUBLE_CLICK_MS  300     // Double click window
#define BUTTON_REPEAT_MS        200     // Repeat rate for volume buttons
#define BUTTON_COMBO_TIMEOUT_MS 500     // Combo button timeout

// ====================================================================================
// AUDIO CONFIGURATION
// ====================================================================================

// Audio Processing
#define AUDIO_SAMPLE_RATE       16000   // 16kHz for voice processing
#define AUDIO_BUFFER_SIZE       512     // Audio buffer size in samples
#define AUDIO_FRAME_SIZE        160     // 10ms frames at 16kHz
#define AUDIO_CHANNELS          1       // Mono input for mic
#define AUDIO_BITS_PER_SAMPLE   16      // 16-bit samples

// Voice Activity Detection (VAD)
#define VAD_THRESHOLD           0.02f   // Voice activity threshold
#define VAD_HANGOVER_MS         500     // Hangover time after voice stops
#define VAD_TRIGGER_MS          50      // Time to trigger VAD
#define VAD_ENERGY_ALPHA        0.1f    // Energy smoothing factor
#define VAD_ZCR_THRESHOLD       0.1f    // Zero crossing rate threshold

// Noise Suppression
#define NOISE_GATE_ATTACK_MS    5       // Noise gate attack time
#define NOISE_GATE_RELEASE_MS   50      // Noise gate release time
#define NOISE_FLOOR_ALPHA       0.95f   // Noise floor adaptation rate
#define SPECTRAL_FLOOR          0.1f    // Minimum spectral gain
#define NOISE_REDUCTION_LEVEL   0.7f    // Default noise reduction level

// Audio Effects
#define AGC_TARGET_LEVEL        0.5f    // AGC target level
#define AGC_ATTACK_MS           1       // AGC attack time
#define AGC_RELEASE_MS          100     // AGC release time
#define AGC_MAX_GAIN            10.0f   // Maximum AGC gain
#define AGC_MIN_GAIN            0.1f    // Minimum AGC gain

// ====================================================================================
// BLUETOOTH CONFIGURATION
// ====================================================================================

#define BT_DEVICE_NAME          "ESP32-C3-Headset"
#define BT_PIN_CODE             "0000"
#define BT_PAIRING_TIMEOUT_MS   120000  // 2 minutes pairing timeout
#define BT_RECONNECT_ATTEMPTS   5       // Reconnection attempts
#define BT_RECONNECT_DELAY_MS   5000    // Delay between reconnection attempts

// ====================================================================================
// POWER MANAGEMENT
// ====================================================================================

#define POWER_SAVE_TIMEOUT_MS   300000  // 5 minutes idle timeout
#define POWER_DEEP_SLEEP_MS     600000  // 10 minutes deep sleep timeout
#define POWER_WAKE_ON_BUTTON    true    // Wake on button press
#define POWER_WAKE_ON_AUDIO     true    // Wake on audio activity
#define POWER_CPU_FREQ_ACTIVE   160     // Active CPU frequency (MHz)
#define POWER_CPU_FREQ_IDLE     80      // Idle CPU frequency (MHz)

// ====================================================================================
// SYSTEM TIMING
// ====================================================================================

#define MAIN_LOOP_DELAY_MS      10      // Main loop delay
#define TASK_STACK_SIZE         4096    // Default task stack size
#define TASK_PRIORITY_HIGH      5       // High priority tasks
#define TASK_PRIORITY_NORMAL    3       // Normal priority tasks
#define TASK_PRIORITY_LOW       1       // Low priority tasks

// Queue sizes
#define BUTTON_QUEUE_SIZE       10      // Button event queue
#define AUDIO_QUEUE_SIZE        4       // Audio processing queue
#define DISPLAY_QUEUE_SIZE      5       // Display update queue

// ====================================================================================
// USB HID CONFIGURATION
// ====================================================================================

#define USB_HID_ENABLED         true    // Enable USB HID functionality
#define USB_VENDOR_ID           0x1234  // USB Vendor ID
#define USB_PRODUCT_ID          0x5678  // USB Product ID
#define USB_MANUFACTURER_STRING "ESP32-C3 Audio"
#define USB_PRODUCT_STRING      "Bluetooth Headset"

// Teams/Discord keyboard shortcuts
#define TEAMS_MUTE_COMBO        "ctrl+shift+m"
#define DISCORD_MUTE_COMBO      "ctrl+shift+m"
#define TEAMS_VIDEO_COMBO       "ctrl+shift+o"
#define DISCORD_DEAFEN_COMBO    "ctrl+shift+d"

// ====================================================================================
// DEBUG AND LOGGING
// ====================================================================================

#define DEBUG_ENABLED           true    // Enable debug output
#define DEBUG_LEVEL             3       // Debug level (0-4)
#define DEBUG_SERIAL_SPEED      115200  // Serial debug speed
#define DEBUG_AUDIO_STATS       false   // Enable audio statistics
#define DEBUG_BATTERY_STATS     false   // Enable battery statistics
#define DEBUG_BUTTON_EVENTS     false   // Enable button event logging

// Debug levels
#define DEBUG_LEVEL_NONE        0       // No debug output
#define DEBUG_LEVEL_ERROR       1       // Error messages only
#define DEBUG_LEVEL_WARNING     2       // Warnings and errors
#define DEBUG_LEVEL_INFO        3       // Info, warnings, and errors
#define DEBUG_LEVEL_DEBUG       4       // All debug output

// ====================================================================================
// FEATURE ENABLES
// ====================================================================================

#define FEATURE_OLED_DISPLAY    true    // Enable OLED display
#define FEATURE_BATTERY_MONITOR true    // Enable battery monitoring
#define FEATURE_TEMPERATURE     false   // Enable temperature monitoring
#define FEATURE_NOISE_REDUCTION true    // Enable noise reduction
#define FEATURE_VAD             true    // Enable voice activity detection
#define FEATURE_AGC             true    // Enable automatic gain control
#define FEATURE_EQUALIZER       false   // Enable equalizer (resource intensive)
#define FEATURE_USB_HID         true    // Enable USB HID functionality
#define FEATURE_BLUETOOTH       true    // Enable Bluetooth functionality
#define FEATURE_SLEEP_MODE      true    // Enable sleep mode

// ====================================================================================
// MEMORY CONFIGURATION
// ====================================================================================

#define HEAP_SIZE_MIN           50000   // Minimum heap size (bytes)
#define STACK_SIZE_MAIN         8192    // Main task stack size
#define STACK_SIZE_AUDIO        8192    // Audio task stack size
#define STACK_SIZE_DISPLAY      4096    // Display task stack size
#define STACK_SIZE_BUTTON       2048    // Button task stack size

// ====================================================================================
// ERROR HANDLING
// ====================================================================================

#define ERROR_RECOVERY_ENABLED  true    // Enable error recovery
#define ERROR_RECOVERY_ATTEMPTS 3       // Number of recovery attempts
#define ERROR_RECOVERY_DELAY_MS 1000    // Delay between recovery attempts
#define WATCHDOG_TIMEOUT_MS     30000   // Watchdog timeout

// ====================================================================================
// CALIBRATION VALUES
// ====================================================================================

// ADC calibration (adjust based on your hardware)
#define ADC_VREF                1100    // ADC reference voltage (mV)
#define ADC_ATTENUATION         ADC_ATTEN_DB_11  // ADC attenuation

// Audio calibration
#define MIC_GAIN_OFFSET         0.0f    // Microphone gain offset (dB)
#define SPEAKER_GAIN_OFFSET     0.0f    // Speaker gain offset (dB)
#define FREQUENCY_RESPONSE_COMP true    // Enable frequency response compensation

// ====================================================================================
// SAFETY LIMITS
// ====================================================================================

#define MAX_VOLUME_LEVEL        90      // Maximum volume level (0-100)
#define MIN_VOLUME_LEVEL        0       // Minimum volume level
#define MAX_TEMPERATURE         85      // Maximum operating temperature (°C)
#define MIN_TEMPERATURE         -20     // Minimum operating temperature (°C)
#define MAX_BATTERY_VOLTAGE     4.5f    // Maximum battery voltage (safety)
#define MIN_BATTERY_VOLTAGE     2.5f    // Minimum battery voltage (safety)

// ====================================================================================
// UTILITY MACROS
// ====================================================================================

// Debug printing macros
#if DEBUG_ENABLED
#define DEBUG_PRINT(level, fmt, ...) \
    do { \
        if (level <= DEBUG_LEVEL) { \
            Serial.printf("[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)
#else
#define DEBUG_PRINT(level, fmt, ...)
#endif

#define DEBUG_ERROR(fmt, ...)   DEBUG_PRINT(DEBUG_LEVEL_ERROR, "ERROR: " fmt, ##__VA_ARGS__)
#define DEBUG_WARN(fmt, ...)    DEBUG_PRINT(DEBUG_LEVEL_WARNING, "WARN: " fmt, ##__VA_ARGS__)
#define DEBUG_INFO(fmt, ...)    DEBUG_PRINT(DEBUG_LEVEL_INFO, "INFO: " fmt, ##__VA_ARGS__)
#define DEBUG_DEBUG(fmt, ...)   DEBUG_PRINT(DEBUG_LEVEL_DEBUG, "DEBUG: " fmt, ##__VA_ARGS__)

// Utility macros
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))
#define CLAMP(val, min, max)    (MIN(MAX(val, min), max))

// Timing macros
#define MILLIS_TO_TICKS(ms)     pdMS_TO_TICKS(ms)
#define TICKS_TO_MILLIS(ticks)  ((ticks) * portTICK_PERIOD_MS)

// Memory macros
#define SAFE_FREE(ptr)          do { if (ptr) { free(ptr); ptr = nullptr; } } while(0)
#define SAFE_DELETE(ptr)        do { if (ptr) { delete ptr; ptr = nullptr; } } while(0)

#endif // CONFIG_H
