/**
 * System Utilities Library
 * 
 * This library provides various utility functions for system management,
 * error handling, diagnostics, and helper functions.
 * 
 * Author: ESP32-C3 Audio Team
 * Date: July 2025
 */

#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <Arduino.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <config.h>

// ====================================================================================
// SYSTEM DIAGNOSTICS
// ====================================================================================

class SystemDiagnostics {
public:
    struct SystemInfo {
        uint32_t freeHeap;
        uint32_t minFreeHeap;
        uint32_t largestFreeBlock;
        uint32_t totalPSRAM;
        uint32_t freePSRAM;
        uint32_t uptime;
        uint32_t cpuFreqMHz;
        float temperature;
        uint8_t cpuUsage;
        uint16_t taskCount;
    };
    
    struct AudioStats {
        uint32_t samplesProcessed;
        uint32_t droppedFrames;
        uint32_t vadTriggers;
        float noiseFloor;
        float signalLevel;
        float snrDb;
        uint32_t processingTime;
    };
    
    struct BatteryStats {
        float voltage;
        uint8_t percentage;
        bool isCharging;
        uint32_t chargeTime;
        uint32_t dischargeTime;
        float averageCurrent;
        uint32_t cycleCount;
    };
    
    static void begin();
    static SystemInfo getSystemInfo();
    static AudioStats getAudioStats();
    static BatteryStats getBatteryStats();
    static void updateStats();
    static void printSystemInfo();
    static void printMemoryUsage();
    static void printTaskList();
    static void enableWatchdog(uint32_t timeoutMs);
    static void feedWatchdog();
    static void resetStats();
    
private:
    static SystemInfo systemInfo;
    static AudioStats audioStats;
    static BatteryStats batteryStats;
    static uint32_t lastUpdateTime;
    static bool watchdogEnabled;
};

// ====================================================================================
// ERROR HANDLING
// ====================================================================================

class ErrorHandler {
public:
    enum ErrorCode {
        ERROR_NONE = 0,
        ERROR_INIT_FAILED,
        ERROR_HARDWARE_FAULT,
        ERROR_MEMORY_ALLOCATION,
        ERROR_BLUETOOTH_FAILURE,
        ERROR_AUDIO_FAILURE,
        ERROR_DISPLAY_FAILURE,
        ERROR_BATTERY_CRITICAL,
        ERROR_TEMPERATURE_HIGH,
        ERROR_WATCHDOG_TIMEOUT,
        ERROR_SYSTEM_CRASH,
        ERROR_CUSTOM
    };
    
    struct ErrorInfo {
        ErrorCode code;
        const char* message;
        const char* function;
        uint32_t line;
        uint32_t timestamp;
        bool fatal;
    };
    
    static void begin();
    static void handleError(ErrorCode code, const char* message, const char* function, uint32_t line, bool fatal = false);
    static void logError(ErrorCode code, const char* message);
    static void clearErrors();
    static uint8_t getErrorCount();
    static ErrorInfo getLastError();
    static void printErrorHistory();
    static void setErrorCallback(void (*callback)(ErrorInfo error));
    
private:
    static ErrorInfo errorHistory[10];
    static uint8_t errorCount;
    static void (*errorCallback)(ErrorInfo error);
    static void handleFatalError(ErrorInfo error);
    static void recoverFromError(ErrorInfo error);
};

// Error handling macros
#define HANDLE_ERROR(code, msg) ErrorHandler::handleError(code, msg, __func__, __LINE__)
#define HANDLE_FATAL_ERROR(code, msg) ErrorHandler::handleError(code, msg, __func__, __LINE__, true)

// ====================================================================================
// POWER MANAGEMENT
// ====================================================================================

class PowerManager {
public:
    enum PowerState {
        POWER_STATE_ACTIVE,
        POWER_STATE_IDLE,
        POWER_STATE_LIGHT_SLEEP,
        POWER_STATE_DEEP_SLEEP,
        POWER_STATE_SHUTDOWN
    };
    
    static void begin();
    static void setPowerState(PowerState state);
    static PowerState getPowerState();
    static void enableComponent(const char* component, bool enable);
    static void setCpuFrequency(uint32_t frequencyMHz);
    static uint32_t getCpuFrequency();
    static void scheduleWakeup(uint32_t delayMs);
    static void configureSleepMode();
    static void enterSleepMode();
    static void wakeFromSleep();
    static uint32_t getUptimeMs();
    static float getPowerConsumption();
    static void enablePowerSaving(bool enable);
    
private:
    static PowerState currentState;
    static uint32_t bootTime;
    static bool powerSavingEnabled;
    static void configureWakeupSources();
    static void savePowerState();
    static void restorePowerState();
};

// ====================================================================================
// CONFIGURATION MANAGER
// ====================================================================================

class ConfigManager {
public:
    struct Config {
        // Audio settings
        uint8_t volumeLevel;
        bool noiseReductionEnabled;
        bool agcEnabled;
        float vadThreshold;
        
        // Display settings
        uint8_t displayBrightness;
        uint32_t displayTimeout;
        bool displayEnabled;
        
        // Power settings
        uint32_t sleepTimeout;
        bool powerSavingEnabled;
        
        // Bluetooth settings
        char deviceName[32];
        char pinCode[8];
        bool autoConnect;
        
        // System settings
        uint8_t debugLevel;
        bool statusLedEnabled;
        
        // User preferences
        uint8_t buttonSensitivity;
        bool doubleClickEnabled;
        uint32_t longPressTime;
        
        // Calibration values
        float batteryCalibration;
        float micGainOffset;
        float tempOffset;
    };
    
    static void begin();
    static bool loadConfig();
    static bool saveConfig();
    static void resetToDefaults();
    static Config& getConfig();
    static void setConfig(const Config& config);
    static void printConfig();
    static bool isConfigValid();
    
private:
    static Config currentConfig;
    static void setDefaultConfig();
    static uint32_t calculateChecksum(const Config& config);
    static bool validateConfig(const Config& config);
};

// ====================================================================================
// UTILITY FUNCTIONS
// ====================================================================================

class Utils {
public:
    // String utilities
    static String formatTime(uint32_t seconds);
    static String formatBytes(uint32_t bytes);
    static String formatFloat(float value, uint8_t decimals);
    static String formatPercent(float value);
    static bool isValidString(const char* str, uint8_t maxLength);
    
    // Math utilities
    static float mapFloat(float value, float inMin, float inMax, float outMin, float outMax);
    static float clampFloat(float value, float min, float max);
    static uint32_t nextPowerOf2(uint32_t value);
    static float dbToLinear(float db);
    static float linearToDb(float linear);
    static float rmsToDb(float rms);
    
    // Array utilities
    static float calculateMean(float* array, uint16_t size);
    static float calculateStdDev(float* array, uint16_t size);
    static void smoothArray(float* array, uint16_t size, float alpha);
    static uint16_t findPeak(float* array, uint16_t size);
    static void normalizeArray(float* array, uint16_t size);
    
    // CRC utilities
    static uint16_t calculateCRC16(const uint8_t* data, uint16_t length);
    static uint32_t calculateCRC32(const uint8_t* data, uint16_t length);
    static bool verifyCRC16(const uint8_t* data, uint16_t length, uint16_t expectedCRC);
    static bool verifyCRC32(const uint8_t* data, uint16_t length, uint32_t expectedCRC);
    
    // Timing utilities
    static uint32_t getTimestamp();
    static uint32_t getElapsedTime(uint32_t startTime);
    static void delayMs(uint32_t ms);
    static void delayUs(uint32_t us);
    static bool isTimeout(uint32_t startTime, uint32_t timeoutMs);
    
    // Random utilities
    static uint32_t random32();
    static float randomFloat(float min, float max);
    static void randomSeed(uint32_t seed);
    
    // Hardware utilities
    static float readTemperature();
    static uint32_t getChipId();
    static String getMacAddress();
    static uint32_t getFreeHeap();
    static uint32_t getUsedHeap();
    static void restart();
    static void factoryReset();
    
    // Debug utilities
    static void hexDump(const uint8_t* data, uint16_t length);
    static void printStackTrace();
    static void printHeapInfo();
    static void printSystemInfo();
    static void enableCoreDebug(bool enable);
};

// ====================================================================================
// PERFORMANCE MONITOR
// ====================================================================================

class PerformanceMonitor {
public:
    struct TaskStats {
        char name[16];
        uint32_t runtime;
        uint32_t stackHighWaterMark;
        uint8_t priority;
        uint8_t cpuUsage;
        bool isRunning;
    };
    
    static void begin();
    static void startTimer(const char* name);
    static void stopTimer(const char* name);
    static void markExecution(const char* name);
    static float getAverageTime(const char* name);
    static float getMaxTime(const char* name);
    static void printPerformanceReport();
    static void resetCounters();
    static TaskStats getTaskStats(const char* taskName);
    static void printTaskStats();
    static uint8_t getCpuUsage();
    static void enableProfiling(bool enable);
    
private:
    struct TimerData {
        char name[16];
        uint32_t startTime;
        uint32_t totalTime;
        uint32_t count;
        uint32_t maxTime;
        bool active;
    };
    
    static TimerData timers[10];
    static uint8_t timerCount;
    static bool profilingEnabled;
    static uint32_t lastCpuTime;
    static uint32_t lastIdleTime;
    
    static TimerData* findTimer(const char* name);
    static TimerData* addTimer(const char* name);
};

// Performance monitoring macros
#define PERF_START(name) PerformanceMonitor::startTimer(name)
#define PERF_STOP(name) PerformanceMonitor::stopTimer(name)
#define PERF_MARK(name) PerformanceMonitor::markExecution(name)

#endif // SYSTEM_UTILS_H
