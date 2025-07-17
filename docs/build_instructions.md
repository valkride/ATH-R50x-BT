# ESP32-C3 Bluetooth Headset Module Build Instructions

## Prerequisites

### Required Software
- **ESP-IDF v5.0 or later**: [Installation Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/index.html)
- **Python 3.8+**: Required for ESP-IDF
- **Git**: For version control and ESP-IDF installation
- **CMake**: Build system (comes with ESP-IDF)
- **Serial Driver**: For programming the ESP32-C3

### Hardware Requirements
- ESP32-C3 development board or custom PCB
- USB-C cable for programming and power
- Audio output device (headphones or speakers)
- Test equipment (multimeter, oscilloscope - optional)

## Environment Setup

### 1. Install ESP-IDF

#### Windows
```cmd
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
install.bat
export.bat
```

#### Linux/macOS
```bash
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh
```

### 2. Verify Installation
```bash
idf.py --version
```

### 3. Set Target Chip
```bash
idf.py set-target esp32c3
```

## Project Setup

### 1. Clone Repository
```bash
git clone <repository-url>
cd BT-module-ATH-R50x/Build
```

### 2. Configure Project
```bash
idf.py menuconfig
```

#### Key Configuration Options:
- **Bluetooth Headset Configuration**
  - Device name: "ATH-R50x BT Module"
  - PIN code: "0000"
  - Auto-reconnect: Enable
  
- **Audio Configuration**
  - Sample rate: 44100 Hz
  - Bits per sample: 16
  - Channels: 2 (stereo)
  - Buffer size: 1024 samples
  
- **GPIO Configuration**
  - I2S BCK: GPIO4
  - I2S WS: GPIO5
  - I2S DATA: GPIO6
  - Buttons: GPIO0, GPIO1, GPIO2
  - Status LED: GPIO8
  - Battery ADC: GPIO3
  
- **Power Management**
  - Sleep timeout: 300 seconds
  - Low battery threshold: 3400 mV
  - Shutdown threshold: 3200 mV

### 3. Hardware Connections

#### I2S Audio (PCM5102A)
| ESP32-C3 Pin | PCM5102A Pin | Function |
|--------------|--------------|----------|
| GPIO4        | BCK          | Bit Clock |
| GPIO5        | LCK          | Word Select |
| GPIO6        | DIN          | Data Input |
| 3.3V         | VCC          | Power |
| GND          | GND          | Ground |

#### User Interface
| ESP32-C3 Pin | Component | Function |
|--------------|-----------|----------|
| GPIO0        | Button    | Play/Pause |
| GPIO1        | Button    | Volume Up |
| GPIO2        | Button    | Volume Down |
| GPIO8        | LED       | Status Indicator |

#### Power Management
| ESP32-C3 Pin | Component | Function |
|--------------|-----------|----------|
| GPIO3        | Voltage Divider | Battery Monitoring |

## Build Process

### 1. Clean Build
```bash
idf.py clean
```

### 2. Build Project
```bash
idf.py build
```

### 3. Flash Firmware
```bash
idf.py flash
```

### 4. Monitor Output
```bash
idf.py monitor
```

### 5. Combined Flash and Monitor
```bash
idf.py flash monitor
```

## Build Output

### Generated Files
- `build/bt_headset_module.bin`: Main firmware binary
- `build/bootloader/bootloader.bin`: Bootloader binary
- `build/partition_table/partition-table.bin`: Partition table
- `build/bt_headset_module.elf`: ELF file with debug symbols
- `build/bt_headset_module.map`: Memory map file

### Memory Usage
The build process will display memory usage:
```
Memory usage:
- DRAM: XXX KB / 400 KB
- Flash: XXX KB / 4096 KB
```

## Testing

### 1. Hardware Test
```bash
# Flash and monitor
idf.py flash monitor

# Expected output:
# I (xxx) MAIN: Starting ESP32-C3 Bluetooth Headset Module v1.0.0
# I (xxx) POWER_MANAGER: Battery voltage: 3800 mV (75%)
# I (xxx) BT_MANAGER: Bluetooth manager initialized successfully
# I (xxx) MAIN: System ready - entering main loop
```

### 2. Bluetooth Pairing Test
1. Power on the device
2. Enable Bluetooth on phone/computer
3. Look for "ATH-R50x BT Module" in available devices
4. Pair with PIN "0000"
5. Test audio playback

### 3. Audio Quality Test
1. Play test audio (sine wave, music)
2. Verify no distortion or dropouts
3. Test volume control
4. Check both channels (L/R)

### 4. Button Test
1. Test play/pause functionality
2. Test volume up/down
3. Verify LED status indications
4. Test button response time

### 5. Battery Test
1. Monitor battery voltage readings
2. Test low battery warnings
3. Verify sleep mode operation
4. Check charging functionality (if implemented)

## Debugging

### 1. Serial Debug Output
```bash
idf.py monitor

# Use Ctrl+] to exit monitor
```

### 2. Enable Verbose Logging
In `menuconfig`:
- Component config → Log output → Default log verbosity → Debug

### 3. GDB Debugging
```bash
idf.py openocd &
xtensa-esp32c3-elf-gdb build/bt_headset_module.elf
```

### 4. Core Dump Analysis
```bash
idf.py coredump-info
```

## Troubleshooting

### Common Issues

#### 1. Build Errors
```
Error: Command failed: idf.py build
```
**Solution**: Check ESP-IDF environment setup and dependencies

#### 2. Flash Errors
```
Error: Failed to connect to ESP32-C3
```
**Solution**: 
- Check USB connection
- Press and hold BOOT button while connecting
- Verify correct COM port

#### 3. No Audio Output
**Solution**:
- Check I2S connections
- Verify PCM5102A power supply
- Check audio cable connections

#### 4. Bluetooth Connection Issues
**Solution**:
- Clear paired devices list
- Reset Bluetooth on source device
- Check PIN code configuration

#### 5. High Power Consumption
**Solution**:
- Verify sleep mode configuration
- Check for busy loops in code
- Monitor task CPU usage

### Advanced Debugging

#### 1. Memory Leak Detection
```bash
idf.py menuconfig
# Component config → ESP System Settings → Memory → Enable heap tracing
```

#### 2. Task Monitoring
```bash
# In monitor, use runtime stats
vTaskList()
vTaskGetRunTimeStats()
```

#### 3. Performance Analysis
```bash
# Enable profiling in menuconfig
# Component config → ESP System Settings → Enable profiling
```

## Optimization

### 1. Code Size Optimization
```bash
idf.py menuconfig
# Compiler options → Optimization Level → Optimize for size (-Os)
```

### 2. Performance Optimization
```bash
idf.py menuconfig
# Compiler options → Optimization Level → Optimize for performance (-O2)
```

### 3. Power Optimization
- Enable light sleep mode
- Optimize task priorities
- Reduce unnecessary wake-ups
- Use appropriate CPU frequency

## Deployment

### 1. Production Build
```bash
# Set optimization level
idf.py menuconfig
# Compiler options → Optimization Level → Optimize for size (-Os)

# Disable debug features
# Component config → Log output → Default log verbosity → None

# Build
idf.py build
```

### 2. Mass Production
```bash
# Generate factory image
idf.py build
esptool.py --chip esp32c3 merge_bin -o factory.bin 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/bt_headset_module.bin
```

### 3. Quality Assurance
- Automated testing scripts
- Audio quality verification
- Range testing
- Battery life testing
- Temperature testing

## Maintenance

### 1. Version Control
```bash
# Tag releases
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

### 2. Documentation Updates
- Update README.md
- Update API documentation
- Update hardware guides
- Update troubleshooting guides

### 3. Backup Configuration
```bash
# Backup current configuration
cp sdkconfig sdkconfig.backup
```

## Support

### Resources
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-C3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf)
- [Bluetooth Audio Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html)

### Getting Help
- Check troubleshooting section
- Review ESP-IDF GitHub issues
- ESP32 community forums
- Hardware manufacturer support
