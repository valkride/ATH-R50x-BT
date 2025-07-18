#!/bin/bash
# ESP32-C3 Firmware Flash Script for Linux/macOS
# Usage: ./flash_firmware.sh [PORT]

# Default port
PORT=${1:-/dev/ttyUSB0}

echo "🔧 ESP32-C3 Headset Firmware Flash Script"
echo "=========================================="
echo "Using port: $PORT"
echo

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "❌ ERROR: PlatformIO CLI not found."
    echo "Please install PlatformIO Core:"
    echo "https://platformio.org/install/cli"
    exit 1
fi

# Check if port exists
if [[ ! -e "$PORT" ]]; then
    echo "⚠️  WARNING: Port $PORT not found."
    echo "Available ports:"
    ls /dev/tty* 2>/dev/null | grep -E "(USB|ACM)" || echo "No USB/ACM ports found"
    echo
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Flash the firmware
echo "🚀 Starting flash process..."
pio run -t upload --upload-port "$PORT"

if [ $? -eq 0 ]; then
    echo
    echo "✅ Firmware flashed successfully!"
    echo "Your ESP32-C3 headset is ready to use."
else
    echo
    echo "❌ Flash failed. Check connections and try again."
    exit 1
fi
