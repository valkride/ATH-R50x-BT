# Windows Build and Flash Guide

## � Easy Way: Download Pre-built Firmware

The simplest way to get started:

1. **Download**: Go to [Releases](https://github.com/YourUsername/esp32-c3-bluetooth-headset/releases)
2. **Extract**: Unzip the downloaded file
3. **Flash**: Double-click `flash_firmware.bat` or run `flash_firmware.ps1` in PowerShell
4. **Follow**: The on-screen instructions

## 🔧 Build from Source (Advanced)

### Prerequisites
- [Python 3.7+](https://www.python.org/downloads/)
- [PlatformIO](https://platformio.org/install/cli)

### Quick Setup
```cmd
# Clone and build
git clone https://github.com/YourUsername/esp32-c3-bluetooth-headset.git
cd esp32-c3-bluetooth-headset
pip install platformio
pio run

# Flash to device
pio run --target upload
```

## 📱 Flashing Instructions

### Step 1: Prepare Hardware
1. Connect ESP32-C3 SuperMini to USB
2. Hold **BOOT** button
3. Press and release **RESET** button
4. Release **BOOT** button
5. Device is now in download mode

### Step 2: Flash Firmware

#### Option A: Use Batch Script (Easiest)
```cmd
flash_firmware.bat COM3
```

#### Option B: Use PowerShell Script
```powershell
.\flash_firmware.ps1 -Port COM3
```

#### Option C: Manual Flash
```cmd
esptool.py --chip esp32c3 --port COM3 write_flash 0x0000 esp32-c3-bluetooth-headset.bin
```

### Step 3: Verify
1. Press **RESET** button on ESP32-C3
2. Check OLED display for "ESP32-C3 Headset"
3. Look for Bluetooth device "ESP32-C3 Headset"

## 🐛 Troubleshooting

**Can't find COM port?**
- Open Device Manager
- Look under "Ports (COM & LPT)"
- Try different USB cable/port

**Flash failed?**
- Ensure device is in download mode
- Try different COM port
- Run as Administrator
- Use lower baud rate: add `--baud 115200` to esptool command

**esptool not found?**
```cmd
pip install esptool
```

## � What You Get

When you download a release:
- `esp32-c3-bluetooth-headset.bin` - Main firmware
- `flash_firmware.bat` - Windows batch script
- `flash_firmware.ps1` - PowerShell script  
- `build_info.txt` - Build information

## 🎯 Quick Start Checklist

- [ ] Download latest release
- [ ] Extract files
- [ ] Connect ESP32-C3 to USB
- [ ] Put in download mode (BOOT + RESET)
- [ ] Run `flash_firmware.bat`
- [ ] Press RESET when done
- [ ] Connect via Bluetooth

That's it! Your ESP32-C3 Bluetooth headset is ready to use.
