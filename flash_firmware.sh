#!/bin/bash

# ESP32-C3 Bluetooth Headset Firmware Flashing Script
# Usage: ./flash_firmware.sh [COM_PORT] [FIRMWARE_FILE]

set -e

# Configuration
CHIP="esp32c3"
BAUD_RATE="921600"
FLASH_MODE="dio"
FLASH_FREQ="80m"
FLASH_SIZE="4MB"

# Default values
DEFAULT_PORT="/dev/ttyUSB0"  # Linux default
DEFAULT_FIRMWARE="esp32-c3-bluetooth-headset.bin"

# Check if running on Windows
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    DEFAULT_PORT="COM3"  # Windows default
fi

# Parse arguments
PORT=${1:-$DEFAULT_PORT}
FIRMWARE=${2:-$DEFAULT_FIRMWARE}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}ESP32-C3 SuperMini Bluetooth Headset Firmware Flasher${NC}"
echo -e "${BLUE}====================================================${NC}"
echo ""

# Check if firmware file exists
if [[ ! -f "$FIRMWARE" ]]; then
    echo -e "${RED}Error: Firmware file '$FIRMWARE' not found!${NC}"
    echo "Please download the firmware from GitHub releases or build it yourself."
    exit 1
fi

# Check if esptool is installed
if ! command -v esptool.py &> /dev/null; then
    echo -e "${YELLOW}Warning: esptool.py not found. Attempting to install...${NC}"
    
    # Try to install esptool
    if command -v pip &> /dev/null; then
        pip install esptool
    elif command -v pip3 &> /dev/null; then
        pip3 install esptool
    else
        echo -e "${RED}Error: pip not found. Please install Python and pip first.${NC}"
        echo "Visit: https://www.python.org/downloads/"
        exit 1
    fi
fi

# Show firmware info
echo -e "${GREEN}Firmware Information:${NC}"
echo "  File: $FIRMWARE"
echo "  Size: $(stat -c%s "$FIRMWARE" 2>/dev/null || stat -f%z "$FIRMWARE" 2>/dev/null || echo "unknown") bytes"
echo "  Port: $PORT"
echo "  Chip: $CHIP"
echo ""

# Check if port exists (basic check)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [[ ! -e "$PORT" ]]; then
        echo -e "${YELLOW}Warning: Port '$PORT' may not exist. Available ports:${NC}"
        ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "No serial ports found"
        echo ""
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo ""
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
fi

echo -e "${YELLOW}Ready to flash firmware to ESP32-C3...${NC}"
echo ""
echo "Please ensure:"
echo "1. ESP32-C3 is connected to $PORT"
echo "2. ESP32-C3 is in download mode (hold BOOT button while pressing RESET)"
echo "3. No other programs are using the serial port"
echo ""
read -p "Press Enter to continue or Ctrl+C to cancel..."

echo ""
echo -e "${BLUE}Starting firmware flash...${NC}"

# Flash the firmware
esptool.py \
    --chip $CHIP \
    --port $PORT \
    --baud $BAUD_RATE \
    --before default_reset \
    --after hard_reset \
    write_flash \
    --flash_mode $FLASH_MODE \
    --flash_freq $FLASH_FREQ \
    --flash_size $FLASH_SIZE \
    0x0000 "$FIRMWARE"

if [[ $? -eq 0 ]]; then
    echo ""
    echo -e "${GREEN}✓ Firmware flashed successfully!${NC}"
    echo ""
    echo -e "${GREEN}Next steps:${NC}"
    echo "1. Disconnect and reconnect the ESP32-C3"
    echo "2. The device should start in Bluetooth headset mode"
    echo "3. Look for 'ESP32-C3 Headset' in your Bluetooth devices"
    echo "4. Check the OLED display for status information"
    echo ""
    echo -e "${BLUE}Troubleshooting:${NC}"
    echo "- If it doesn't work, try pressing the RESET button"
    echo "- Check serial monitor at 115200 baud for debug messages"
    echo "- Ensure all hardware connections are correct"
else
    echo ""
    echo -e "${RED}✗ Firmware flash failed!${NC}"
    echo ""
    echo -e "${YELLOW}Troubleshooting:${NC}"
    echo "1. Check if the correct port is selected"
    echo "2. Ensure ESP32-C3 is in download mode"
    echo "3. Try a different USB cable"
    echo "4. Check if the port is not in use by another application"
    echo "5. Try running with sudo (Linux/Mac) or as Administrator (Windows)"
    exit 1
fi
