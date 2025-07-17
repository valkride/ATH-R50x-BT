/**
 * QCC5124 A2DP Codec Control Library
 * 
 * This library provides control functions for the QCC5124 A2DP codec
 * via I2C register interface. The QCC5124 uses register-based control,
 * not AT commands (which are for generic BT modules like HC-05).
 * 
 * Author: ESP32-C3 Audio Team
 * Date: July 2025
 */

#ifndef QCC5124_CONTROL_H
#define QCC5124_CONTROL_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>

// QCC5124 I2C Configuration
#define QCC5124_I2C_ADDRESS         0x18  // Typical I2C address for QCC5124
#define QCC5124_CHIP_ID_VALUE       0x51  // Expected chip ID value

// QCC5124 Register Definitions
#define QCC5124_REG_CHIP_ID         0x00  // Chip identification
#define QCC5124_REG_POWER_CTRL      0x01  // Power control
#define QCC5124_REG_AUDIO_FORMAT    0x02  // Audio format configuration
#define QCC5124_REG_SAMPLE_RATE     0x03  // Sample rate control
#define QCC5124_REG_VOLUME_CTRL     0x04  // Volume control
#define QCC5124_REG_MUTE_CTRL       0x05  // Mute control
#define QCC5124_REG_BT_CONFIG       0x06  // Bluetooth configuration
#define QCC5124_REG_BT_CTRL         0x07  // Bluetooth control
#define QCC5124_REG_BT_STATUS       0x08  // Bluetooth status
#define QCC5124_REG_PROFILE_CTRL    0x09  // Profile control (A2DP, etc.)
#define QCC5124_REG_AUDIO_ROUTE     0x0A  // Audio routing
#define QCC5124_REG_EQ_PRESET       0x0B  // Equalizer preset
#define QCC5124_REG_CODEC_CONFIG    0x0C  // Codec configuration
#define QCC5124_REG_CODEC_PRIORITY  0x0D  // Codec priority
#define QCC5124_REG_NOISE_REDUCTION 0x0E  // Noise reduction
#define QCC5124_REG_FW_VERSION      0x10  // Firmware version (16-bit)
#define QCC5124_REG_DEVICE_NAME     0x20  // Device name (16 bytes)
#define QCC5124_REG_BATTERY_ADC     0x30  // Battery ADC reading (16-bit)
#define QCC5124_REG_RSSI            0x32  // RSSI reading

// Include reset pin definition
#ifndef PIN_QCC_RST
#define PIN_QCC_RST 2  // Default reset pin
#endif

class QCC5124Control {
private:
    HardwareSerial* uart;  // For debugging output only
    uint8_t currentVolume;
    bool isInitialized;
    bool isConnected;
    uint32_t lastCommandTime;
    uint8_t i2cAddress;
    
    // Command timeout and retry settings
    static const uint32_t COMMAND_TIMEOUT_MS = 1000;
    static const uint8_t MAX_RETRIES = 3;
    static const uint32_t COMMAND_DELAY_MS = 100;
    
    // Volume settings
    static const uint8_t MIN_VOLUME = 0;
    static const uint8_t MAX_VOLUME = 15;
    static const uint8_t DEFAULT_VOLUME = 8;
    
    // I2C register access functions
    bool writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    bool writeRegister16(uint8_t reg, uint16_t value);
    uint16_t readRegister16(uint8_t reg);
    
    // Hardware control
    bool isChipPresent();
    bool initializeRegisters();
    bool enableA2DP();
    bool configureAudioRouting();
    
    // Deprecated AT command functions (kept for compatibility)
    bool sendCommand(const String& command, const String& expectedResponse = "OK");
    String readResponse(uint32_t timeout = COMMAND_TIMEOUT_MS);
    bool waitForResponse(const String& expected, uint32_t timeout = COMMAND_TIMEOUT_MS);
    
public:
    QCC5124Control(HardwareSerial* serialPort);
    
    // Initialization and configuration
    bool begin();
    bool reset();
    bool isReady();
    
    // Audio control
    bool setVolume(uint8_t volume);
    bool volumeUp();
    bool volumeDown();
    uint8_t getVolume();
    bool mute(bool enable);
    
    // Connection management
    bool startPairing();
    bool stopPairing();
    bool disconnect();
    bool getConnectionStatus();
    
    // Audio routing
    bool setAudioRoute(uint8_t route);  // 0=Headphones, 1=Speaker, 2=Both
    bool setEqualizer(uint8_t preset);  // 0=Off, 1=Rock, 2=Pop, 3=Jazz, etc.
    
    // Advanced features
    bool enableAptX(bool enable);
    bool enableNoiseReduction(bool enable);
    bool setCodecPriority(uint8_t priority);  // 0=SBC, 1=AAC, 2=aptX
    
    // Status and diagnostics
    String getFirmwareVersion();
    String getDeviceName();
    bool setDeviceName(const String& name);
    float getBatteryVoltage();
    int8_t getSignalStrength();
    
    // Callback for status updates
    void setStatusCallback(void (*callback)(const String& status));
    void processIncomingData();
    
private:
    void (*statusCallback)(const String& status) = nullptr;
};

#endif // QCC5124_CONTROL_H
