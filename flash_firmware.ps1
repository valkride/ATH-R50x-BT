# ESP32-C3 Bluetooth Headset Firmware Flasher for Windows
# Usage: .\flash_firmware.ps1 [COM_PORT] [FIRMWARE_FILE]

param(
    [string]$Port = "COM3",
    [string]$Firmware = "esp32-c3-bluetooth-headset.bin"
)

$ErrorActionPreference = "Stop"

# Configuration
$CHIP = "esp32c3"
$BAUD_RATE = "921600"
$FLASH_MODE = "dio"
$FLASH_FREQ = "80m"
$FLASH_SIZE = "4MB"

Write-Host "ESP32-C3 SuperMini Bluetooth Headset Firmware Flasher" -ForegroundColor Blue
Write-Host "====================================================" -ForegroundColor Blue
Write-Host ""

# Check if firmware file exists
if (!(Test-Path $Firmware)) {
    Write-Host "Error: Firmware file '$Firmware' not found!" -ForegroundColor Red
    Write-Host "Please download the firmware from GitHub releases or build it yourself."
    Read-Host "Press Enter to exit"
    exit 1
}

# Check if esptool is installed
try {
    esptool.py --help | Out-Null
} catch {
    Write-Host "Warning: esptool.py not found. Attempting to install..." -ForegroundColor Yellow
    
    try {
        python -m pip install esptool
    } catch {
        Write-Host "Error: Failed to install esptool. Please install Python and pip first." -ForegroundColor Red
        Write-Host "Visit: https://www.python.org/downloads/"
        Read-Host "Press Enter to exit"
        exit 1
    }
}

# Show firmware info
Write-Host "Firmware Information:" -ForegroundColor Green
Write-Host "  File: $Firmware"
Write-Host "  Size: $((Get-Item $Firmware).Length) bytes"
Write-Host "  Port: $Port"
Write-Host "  Chip: $CHIP"
Write-Host ""

Write-Host "Ready to flash firmware to ESP32-C3..." -ForegroundColor Yellow
Write-Host ""
Write-Host "Please ensure:"
Write-Host "1. ESP32-C3 is connected to $Port"
Write-Host "2. ESP32-C3 is in download mode (hold BOOT button while pressing RESET)"
Write-Host "3. No other programs are using the serial port"
Write-Host ""
Read-Host "Press Enter to continue or Ctrl+C to cancel"

Write-Host ""
Write-Host "Starting firmware flash..." -ForegroundColor Blue

# Flash the firmware
try {
    & esptool.py --chip $CHIP --port $Port --baud $BAUD_RATE --before default_reset --after hard_reset write_flash --flash_mode $FLASH_MODE --flash_freq $FLASH_FREQ --flash_size $FLASH_SIZE 0x0000 $Firmware
    
    Write-Host ""
    Write-Host "✓ Firmware flashed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Green
    Write-Host "1. Disconnect and reconnect the ESP32-C3"
    Write-Host "2. The device should start in Bluetooth headset mode"
    Write-Host "3. Look for 'ESP32-C3 Headset' in your Bluetooth devices"
    Write-Host "4. Check the OLED display for status information"
    Write-Host ""
    Write-Host "Troubleshooting:" -ForegroundColor Blue
    Write-Host "- If it doesn't work, try pressing the RESET button"
    Write-Host "- Check serial monitor at 115200 baud for debug messages"
    Write-Host "- Ensure all hardware connections are correct"
    
} catch {
    Write-Host ""
    Write-Host "✗ Firmware flash failed!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Troubleshooting:" -ForegroundColor Yellow
    Write-Host "1. Check if the correct port is selected"
    Write-Host "2. Ensure ESP32-C3 is in download mode"
    Write-Host "3. Try a different USB cable"
    Write-Host "4. Check if the port is not in use by another application"
    Write-Host "5. Try running as Administrator"
    Write-Host ""
    Write-Host "Error details: $($_.Exception.Message)" -ForegroundColor Red
}

Read-Host "Press Enter to exit"
