# ESP32-C3 SuperMini Bluetooth Headset Setup Guide

## Quick Start

This firmware provides a complete Arduino-style C++ implementation for your ESP32-C3 SuperMini based split-cup Bluetooth headset.

### Hardware Connections

**Left Cup (ESP32-C3 SuperMini):**
- GPIO21 → SDA (SSD1306 OLED)
- GPIO22 → SCL (SSD1306 OLED)
- GPIO35 → Battery voltage divider (ADC)
- GPIO34 → TP4056 STAT line (from right cup)
- GPIO25 → Audio enable (QCC5124 + TPA6120A2)
- GPIO26 → Mic enable (P-MOSFET gate)
- GPIO27 → Power button
- GPIO14 → Volume up button
- GPIO12 → Volume down button
- GPIO13 → Mute button
- GPIO10 → QCC5124 UART TX
- GPIO9 → QCC5124 UART RX
- GPIO2 → QCC5124 Reset
- GPIO18 → I2S Word Select
- GPIO19 → I2S Serial Clock
- GPIO23 → I2S Serial Data

**Right Cup:**
- TP4056 charger module
- BLE mic module (powered via P-MOSFET from left cup)
- 6-wire flex cable: Vbat, GND, SDA, SCL, EN_MIC, STAT

### Building and Flashing

1. **Install PlatformIO**: 
   ```bash
   pip install platformio
   ```

2. **Build the firmware**:
   ```bash
   cd Build
   pio run
   ```

3. **Flash to ESP32-C3**:
   ```bash
   pio run --target upload
   ```

4. **Monitor serial output**:
   ```bash
   pio device monitor
   ```

### Key Features

✅ **Battery Monitoring**: Shows charging status and percentage on OLED
✅ **Button Controls**: Power, Volume+/-, Mute with debouncing
✅ **Audio Control**: QCC5124 A2DP codec control via UART
✅ **Voice Activity Detection**: VAD with noise gating
✅ **Noise Suppression**: Basic spectral subtraction
✅ **USB HID**: Ctrl+Shift+M for Teams/Discord mute
✅ **Power Management**: Separate audio and mic power rails
✅ **Real-time Processing**: FreeRTOS tasks for audio and display

### Button Functions

- **Power (short press)**: Toggle audio system on/off
- **Power (long press)**: Force shutdown
- **Volume Up/Down**: Send volume commands to QCC5124
- **Mute (short press)**: Toggle mic mute (local)
- **Mute (long press)**: Send Teams/Discord mute command (USB HID)

### System Status

The OLED displays:
- Battery level and charging status
- Audio system state and volume
- Bluetooth connection status
- Voice activity detection state
- Microphone mute status

### Libraries Used

- **Adafruit SSD1306**: OLED display control
- **TinyUSB**: USB HID keyboard functionality
- **ESP32 BLE Arduino**: Bluetooth support
- **ArduinoJson**: Configuration management
- **Custom Libraries**: QCC5124Control, AudioProcessing, SystemUtils

### Advanced Audio Features

- **VAD (Voice Activity Detection)**: Only activates mic when speech is detected
- **Noise Suppression**: Reduces background noise using spectral subtraction
- **Adaptive Filtering**: Real-time noise floor estimation
- **Audio Effects**: AGC, EQ, and other effects via AudioProcessor class

### Troubleshooting

- **No display**: Check I2C connections (GPIO21/22)
- **No audio**: Verify QCC5124 power and UART connections
- **Buttons not working**: Check pullup resistors and GPIO assignments
- **Battery not reading**: Verify ADC connection on GPIO35
- **Mic not working**: Check P-MOSFET gate control on GPIO26

### Customization

Edit `include/config.h` to modify:
- Pin assignments
- Audio parameters
- Battery thresholds
- Button behavior
- Display settings

The firmware is fully modular with separate classes for:
- `QCC5124Control`: Codec control and management
- `AudioProcessor`: VAD and noise suppression
- `SystemUtils`: Diagnostics and utilities
