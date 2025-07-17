# Building and Flashing Guide

## 🔧 Building the Firmware

### Method 1: Using GitHub Actions (Recommended)

The easiest way to get the firmware is to download it from our automated builds:

1. **Releases**: Go to [Releases](https://github.com/YourUsername/esp32-c3-bluetooth-headset/releases) for stable firmware
2. **Nightly Builds**: Go to [Nightly Release](https://github.com/YourUsername/esp32-c3-bluetooth-headset/releases/tag/nightly) for latest features

Each release contains:
- `esp32-c3-bluetooth-headset.bin` - Main firmware binary
- `esp32-c3-bluetooth-headset.elf` - ELF file for debugging
- `firmware_info.txt` - Build information
- Additional files (bootloader, partitions) if needed

### Method 2: Build Locally with PlatformIO

If you want to build the firmware yourself:

#### Prerequisites
- [Python 3.7+](https://www.python.org/downloads/)
- [PlatformIO](https://platformio.org/install/cli)

#### Build Steps
```bash
# Clone the repository
git clone https://github.com/YourUsername/esp32-c3-bluetooth-headset.git
cd esp32-c3-bluetooth-headset

# Install dependencies
pio pkg install

# Build the firmware
pio run

# Build and upload directly to device
pio run --target upload
```

The compiled firmware will be in `.pio/build/esp32-c3-supermini/firmware.bin`

### Method 3: Using Arduino IDE

1. Install ESP32 board support in Arduino IDE
2. Open `src/main.cpp` in Arduino IDE
3. Select board: "ESP32C3 Dev Module"
4. Set the following board options:
   - Flash Size: 4MB
   - Partition Scheme: Default
   - PSRAM: Disabled
5. Install required libraries (see `platformio.ini` for list)
6. Compile and upload

## 📱 Flashing the Firmware

### Automatic Flashing (Recommended)

Use our provided scripts for easy flashing:

#### Windows
```cmd
flash_firmware.bat COM3 esp32-c3-bluetooth-headset.bin
```

#### Linux/Mac
```bash
chmod +x flash_firmware.sh
./flash_firmware.sh /dev/ttyUSB0 esp32-c3-bluetooth-headset.bin
```

### Manual Flashing

If you prefer to flash manually:

#### Method 1: Using esptool (Recommended)

1. **Install esptool**:
   ```bash
   pip install esptool
   ```

2. **Put ESP32-C3 in download mode**:
   - Hold BOOT button
   - Press and release RESET button
   - Release BOOT button

3. **Flash the firmware**:
   ```bash
   esptool.py --chip esp32c3 --port COM3 --baud 921600 write_flash 0x0000 esp32-c3-bluetooth-headset.bin
   ```

#### Method 2: Using ESP Flash Download Tool

1. Download [ESP Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)
2. Select ESP32-C3 and develop mode
3. Configure:
   - File: `esp32-c3-bluetooth-headset.bin`
   - Address: `0x0000`
   - COM port: Your ESP32-C3 port
4. Click START

### Troubleshooting Flash Issues

**Port not found**:
- On Windows: Check Device Manager for COM port
- On Linux: Try `ls /dev/ttyUSB*` or `ls /dev/ttyACM*`
- On Mac: Try `ls /dev/cu.*`

**Flash failed**:
- Ensure ESP32-C3 is in download mode
- Try different USB cable
- Lower baud rate: `--baud 115200`
- Try different port
- Run as Administrator/sudo

**Permission denied (Linux)**:
```bash
sudo usermod -a -G dialout $USER
# Then logout and login again
```

## 🎯 First Boot

After flashing:

1. **Reset the device**: Press the RESET button
2. **Check OLED display**: Should show "ESP32-C3 Headset" and status
3. **Bluetooth pairing**: Look for "ESP32-C3 Headset" in Bluetooth devices
4. **Serial monitor**: Connect at 115200 baud for debug messages

## 📊 Build Status

| Environment | Status | Download |
|-------------|---------|----------|
| Release | [![Build Status](https://github.com/YourUsername/esp32-c3-bluetooth-headset/workflows/Build%20and%20Release%20Firmware/badge.svg)](https://github.com/YourUsername/esp32-c3-bluetooth-headset/actions) | [Latest Release](https://github.com/YourUsername/esp32-c3-bluetooth-headset/releases/latest) |
| Nightly | [![Nightly Build](https://github.com/YourUsername/esp32-c3-bluetooth-headset/workflows/Build%20and%20Release%20Firmware/badge.svg?branch=main)](https://github.com/YourUsername/esp32-c3-bluetooth-headset/actions) | [Nightly Build](https://github.com/YourUsername/esp32-c3-bluetooth-headset/releases/tag/nightly) |

## 🔍 Build Configuration

The firmware is configured in `platformio.ini`:

```ini
[env:esp32-c3-supermini]
platform = espressif32
board = esp32-c3-devkitc-02
framework = arduino
board_build.flash_mode = dio
board_build.flash_size = 4MB
board_build.partitions = default.csv
upload_speed = 921600
monitor_speed = 115200
```

## 🚀 OTA Updates

The firmware supports Over-The-Air (OTA) updates:

1. **Web OTA**: Access the web interface at `http://esp32-headset.local`
2. **Arduino OTA**: Use PlatformIO with `pio run --target upload --upload-port esp32-headset.local`

## 🏗️ Development Build

For development with live reloading:

```bash
# Build and upload with monitoring
pio run --target upload --target monitor

# Or use the watch mode
pio run --target upload && pio device monitor
```

## 📋 Build Dependencies

All dependencies are automatically managed by PlatformIO:

- **ESP32 Arduino Core**: 2.0.17
- **ArduinoJson**: 7.0.4
- **TinyUSB**: 2.0.0
- **Adafruit GFX**: 1.11.9
- **Adafruit SSD1306**: 2.5.10
- **FastLED**: 3.6.0

## 🐛 Debug Build

For debugging, use the debug configuration:

```bash
# Build with debug symbols
pio run -e esp32-c3-supermini-debug

# Or enable debug in platformio.ini
build_type = debug
```

## 📝 Custom Configuration

You can customize the build by modifying `include/config.h`:

```cpp
// Bluetooth configuration
#define BLUETOOTH_NAME "My Custom Headset"
#define BLUETOOTH_PIN "0000"

// Audio configuration
#define SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE 512

// Display configuration
#define OLED_ENABLED true
#define OLED_BRIGHTNESS 128
```

## 🔒 Security

**Important**: The firmware includes security features:
- Secure boot (optional)
- Flash encryption (optional)
- Bluetooth authentication

For production use, consider enabling these features in `platformio.ini`.
