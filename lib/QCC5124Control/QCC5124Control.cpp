/**
 * QCC5124 A2DP Codec Control Library Implementation
 * 
 * This implementation uses I2C for QCC5124 control, which is the actual
 * interface used by this Qualcomm audio chip. AT commands are not supported
 * by QCC5124 - that's for generic Bluetooth modules like HC-05.
 */

#include "QCC5124Control.h"
#include <Wire.h>

QCC5124Control::QCC5124Control(HardwareSerial* serialPort) {
    uart = serialPort;  // Keep for debugging/status output
    currentVolume = DEFAULT_VOLUME;
    isInitialized = false;
    isConnected = false;
    lastCommandTime = 0;
    statusCallback = nullptr;
    i2cAddress = QCC5124_I2C_ADDRESS;
}

bool QCC5124Control::begin() {
    if (!uart) {
        return false;
    }
    
    // Initialize I2C communication
    Wire.begin();
    
    // Reset the codec via hardware reset pin
    if (!reset()) {
        return false;
    }
    
    delay(100);  // Wait for codec to stabilize
    
    // Check if chip is responsive
    if (!isChipPresent()) {
        return false;
    }
    
    // Initialize codec with default settings
    if (!initializeRegisters()) {
        return false;
    }
    
    // Set default volume
    if (!setVolume(DEFAULT_VOLUME)) {
        return false;
    }
    
    // Enable A2DP profile
    if (!enableA2DP()) {
        return false;
    }
    
    // Configure audio routing
    if (!configureAudioRouting()) {
        return false;
    }
    
    isInitialized = true;
    return true;
}

bool QCC5124Control::reset() {
    // Hardware reset via GPIO (assuming PIN_QCC_RST is connected)
    digitalWrite(PIN_QCC_RST, LOW);
    delay(10);
    digitalWrite(PIN_QCC_RST, HIGH);
    delay(100);  // Wait for reset to complete
    
    // Check if chip is responsive after reset
    return isChipPresent();
}

bool QCC5124Control::isChipPresent() {
    // Try to read chip ID register
    uint8_t chipId = readRegister(QCC5124_REG_CHIP_ID);
    return (chipId == QCC5124_CHIP_ID_VALUE);
}

bool QCC5124Control::initializeRegisters() {
    // Initialize QCC5124 registers with default values
    // These are example registers - actual values depend on your specific QCC5124 implementation
    
    // Power management
    if (!writeRegister(QCC5124_REG_POWER_CTRL, 0x01)) return false;
    
    // Audio format configuration
    if (!writeRegister(QCC5124_REG_AUDIO_FORMAT, 0x00)) return false;  // I2S format
    
    // Sample rate configuration (44.1kHz)
    if (!writeRegister(QCC5124_REG_SAMPLE_RATE, 0x00)) return false;
    
    // Bluetooth configuration
    if (!writeRegister(QCC5124_REG_BT_CONFIG, 0x01)) return false;
    
    return true;
}

bool QCC5124Control::enableA2DP() {
    // Enable A2DP profile in the codec
    return writeRegister(QCC5124_REG_PROFILE_CTRL, 0x01);
}

bool QCC5124Control::configureAudioRouting() {
    // Configure audio routing for headphone output
    return writeRegister(QCC5124_REG_AUDIO_ROUTE, 0x01);
}

bool QCC5124Control::isReady() {
    return isInitialized && isChipPresent();
}

bool QCC5124Control::setVolume(uint8_t volume) {
    volume = constrain(volume, MIN_VOLUME, MAX_VOLUME);
    
    // QCC5124 typically uses logarithmic volume control
    // Convert linear volume (0-15) to register value
    uint8_t regValue = (volume * 255) / MAX_VOLUME;
    
    if (writeRegister(QCC5124_REG_VOLUME_CTRL, regValue)) {
        currentVolume = volume;
        return true;
    }
    
    return false;
}

bool QCC5124Control::volumeUp() {
    if (currentVolume < MAX_VOLUME) {
        return setVolume(currentVolume + 1);
    }
    return false;
}

bool QCC5124Control::volumeDown() {
    if (currentVolume > MIN_VOLUME) {
        return setVolume(currentVolume - 1);
    }
    return false;
}

uint8_t QCC5124Control::getVolume() {
    return currentVolume;
}

// ====================================================================================
// I2C REGISTER ACCESS FUNCTIONS
// ====================================================================================

bool QCC5124Control::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    Wire.write(value);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        delay(1);  // Small delay for register write
        return true;
    }
    
    return false;
}

uint8_t QCC5124Control::readRegister(uint8_t reg) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    uint8_t error = Wire.endTransmission(false);  // Don't release bus
    
    if (error != 0) {
        return 0xFF;  // Error value
    }
    
    Wire.requestFrom(i2cAddress, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    
    return 0xFF;  // Error value
}

bool QCC5124Control::writeRegister16(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);  // High byte
    Wire.write(value & 0xFF);         // Low byte
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        delay(1);  // Small delay for register write
        return true;
    }
    
    return false;
}

uint16_t QCC5124Control::readRegister16(uint8_t reg) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    uint8_t error = Wire.endTransmission(false);  // Don't release bus
    
    if (error != 0) {
        return 0xFFFF;  // Error value
    }
    
    Wire.requestFrom(i2cAddress, (uint8_t)2);
    if (Wire.available() >= 2) {
        uint16_t high = Wire.read();
        uint16_t low = Wire.read();
        return (high << 8) | low;
    }
    
    return 0xFFFF;  // Error value
}

// ====================================================================================
// UPDATED CONTROL FUNCTIONS USING I2C
// ====================================================================================

bool QCC5124Control::mute(bool enable) {
    uint8_t muteValue = enable ? 0x01 : 0x00;
    return writeRegister(QCC5124_REG_MUTE_CTRL, muteValue);
}

bool QCC5124Control::startPairing() {
    // Set pairing mode bit in control register
    uint8_t currentState = readRegister(QCC5124_REG_BT_CTRL);
    return writeRegister(QCC5124_REG_BT_CTRL, currentState | 0x01);
}

bool QCC5124Control::stopPairing() {
    // Clear pairing mode bit in control register
    uint8_t currentState = readRegister(QCC5124_REG_BT_CTRL);
    return writeRegister(QCC5124_REG_BT_CTRL, currentState & ~0x01);
}

bool QCC5124Control::disconnect() {
    // Trigger disconnect in control register
    bool result = writeRegister(QCC5124_REG_BT_CTRL, 0x02);
    if (result) {
        isConnected = false;
    }
    return result;
}

bool QCC5124Control::getConnectionStatus() {
    // Read connection status from status register
    uint8_t status = readRegister(QCC5124_REG_BT_STATUS);
    isConnected = (status & 0x01) != 0;
    return isConnected;
}

bool QCC5124Control::setAudioRoute(uint8_t route) {
    return writeRegister(QCC5124_REG_AUDIO_ROUTE, route);
}

bool QCC5124Control::setEqualizer(uint8_t preset) {
    return writeRegister(QCC5124_REG_EQ_PRESET, preset);
}

bool QCC5124Control::enableAptX(bool enable) {
    uint8_t codecConfig = readRegister(QCC5124_REG_CODEC_CONFIG);
    if (enable) {
        codecConfig |= 0x01;  // Enable aptX bit
    } else {
        codecConfig &= ~0x01; // Disable aptX bit
    }
    return writeRegister(QCC5124_REG_CODEC_CONFIG, codecConfig);
}

bool QCC5124Control::enableNoiseReduction(bool enable) {
    uint8_t nrConfig = enable ? 0x01 : 0x00;
    return writeRegister(QCC5124_REG_NOISE_REDUCTION, nrConfig);
}

bool QCC5124Control::setCodecPriority(uint8_t priority) {
    return writeRegister(QCC5124_REG_CODEC_PRIORITY, priority);
}

String QCC5124Control::getFirmwareVersion() {
    uint16_t version = readRegister16(QCC5124_REG_FW_VERSION);
    if (version != 0xFFFF) {
        return String((version >> 8) & 0xFF) + "." + String(version & 0xFF);
    }
    return "Unknown";
}

String QCC5124Control::getDeviceName() {
    // Device name is typically stored in multiple registers
    // This is a simplified implementation
    String name = "";
    for (int i = 0; i < 16; i++) {
        uint8_t nameChar = readRegister(QCC5124_REG_DEVICE_NAME + i);
        if (nameChar == 0 || nameChar == 0xFF) break;
        name += (char)nameChar;
    }
    return name;
}

bool QCC5124Control::setDeviceName(const String& name) {
    // Write device name to registers (simplified implementation)
    for (int i = 0; i < min(16, (int)name.length()); i++) {
        if (!writeRegister(QCC5124_REG_DEVICE_NAME + i, name[i])) {
            return false;
        }
    }
    // Null terminate if name is shorter than 16 chars
    if (name.length() < 16) {
        writeRegister(QCC5124_REG_DEVICE_NAME + name.length(), 0);
    }
    return true;
}

float QCC5124Control::getBatteryVoltage() {
    uint16_t batteryADC = readRegister16(QCC5124_REG_BATTERY_ADC);
    if (batteryADC != 0xFFFF) {
        // Convert ADC value to voltage (example conversion)
        return (float)batteryADC * 0.001f;  // Adjust based on actual ADC reference
    }
    return 0.0f;
}

int8_t QCC5124Control::getSignalStrength() {
    uint8_t rssi = readRegister(QCC5124_REG_RSSI);
    if (rssi != 0xFF) {
        return (int8_t)rssi - 100;  // Convert to dBm
    }
    return -100;  // Invalid signal strength
}

void QCC5124Control::processIncomingData() {
    // Check status registers for changes
    static uint8_t lastStatus = 0;
    uint8_t currentStatus = readRegister(QCC5124_REG_BT_STATUS);
    
    if (currentStatus != lastStatus) {
        // Connection status changed
        bool wasConnected = isConnected;
        isConnected = (currentStatus & 0x01) != 0;
        
        if (statusCallback) {
            if (isConnected && !wasConnected) {
                statusCallback("CONNECTED");
            } else if (!isConnected && wasConnected) {
                statusCallback("DISCONNECTED");
            }
        }
        
        lastStatus = currentStatus;
    }
    
    // Also check for any UART debug messages if available
    if (uart && uart->available()) {
        String data = uart->readString();
        data.trim();
        if (statusCallback && data.length() > 0) {
            statusCallback("DEBUG: " + data);
        }
    }
}

// ====================================================================================
// DEPRECATED AT COMMAND FUNCTIONS (kept for compatibility)
// ====================================================================================

bool QCC5124Control::sendCommand(const String& command, const String& expectedResponse) {
    // This function is deprecated but kept for compatibility
    // In a real QCC5124 implementation, this would not be used
    // Instead, all communication should go through I2C registers
    
    if (uart) {
        uart->println("QCC5124: AT commands not supported, using I2C instead");
        uart->println("Command attempted: " + command);
    }
    
    return false;  // AT commands not supported on QCC5124
}

String QCC5124Control::readResponse(uint32_t timeout) {
    // Deprecated - QCC5124 doesn't use UART responses
    return "";
}

bool QCC5124Control::waitForResponse(const String& expected, uint32_t timeout) {
    // Deprecated - QCC5124 doesn't use UART responses
    return false;
}
