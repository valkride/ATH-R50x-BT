@echo off
setlocal enabledelayedexpansion

REM ESP32-C3 Bluetooth Headset Firmware Flashing Script for Windows
REM Usage: flash_firmware.bat [COM_PORT] [FIRMWARE_FILE]

set "CHIP=esp32c3"
set "BAUD_RATE=921600"
set "FLASH_MODE=dio"
set "FLASH_FREQ=80m"
set "FLASH_SIZE=4MB"

REM Default values
set "DEFAULT_PORT=COM3"
set "DEFAULT_FIRMWARE=esp32-c3-bluetooth-headset.bin"

REM Parse arguments
if "%~1"=="" (
    set "PORT=%DEFAULT_PORT%"
) else (
    set "PORT=%~1"
)

if "%~2"=="" (
    set "FIRMWARE=%DEFAULT_FIRMWARE%"
) else (
    set "FIRMWARE=%~2"
)

echo ESP32-C3 SuperMini Bluetooth Headset Firmware Flasher
echo ====================================================
echo.

REM Check if firmware file exists
if not exist "%FIRMWARE%" (
    echo Error: Firmware file '%FIRMWARE%' not found!
    echo Please download the firmware from GitHub releases or build it yourself.
    pause
    exit /b 1
)

REM Check if esptool is installed
esptool.py --help >nul 2>&1
if errorlevel 1 (
    echo Warning: esptool.py not found. Attempting to install...
    
    REM Try to install esptool
    python -m pip install esptool >nul 2>&1
    if errorlevel 1 (
        echo Error: Failed to install esptool. Please install Python and pip first.
        echo Visit: https://www.python.org/downloads/
        pause
        exit /b 1
    )
)

REM Show firmware info
echo Firmware Information:
echo   File: %FIRMWARE%
for %%A in ("%FIRMWARE%") do echo   Size: %%~zA bytes
echo   Port: %PORT%
echo   Chip: %CHIP%
echo.

echo Ready to flash firmware to ESP32-C3...
echo.
echo Please ensure:
echo 1. ESP32-C3 is connected to %PORT%
echo 2. ESP32-C3 is in download mode (hold BOOT button while pressing RESET)
echo 3. No other programs are using the serial port
echo.
pause

echo.
echo Starting firmware flash...

REM Flash the firmware
esptool.py ^
    --chip %CHIP% ^
    --port %PORT% ^
    --baud %BAUD_RATE% ^
    --before default_reset ^
    --after hard_reset ^
    write_flash ^
    --flash_mode %FLASH_MODE% ^
    --flash_freq %FLASH_FREQ% ^
    --flash_size %FLASH_SIZE% ^
    0x0000 "%FIRMWARE%"

if errorlevel 1 (
    echo.
    echo X Firmware flash failed!
    echo.
    echo Troubleshooting:
    echo 1. Check if the correct port is selected
    echo 2. Ensure ESP32-C3 is in download mode
    echo 3. Try a different USB cable
    echo 4. Check if the port is not in use by another application
    echo 5. Try running as Administrator
    pause
    exit /b 1
) else (
    echo.
    echo √ Firmware flashed successfully!
    echo.
    echo Next steps:
    echo 1. Disconnect and reconnect the ESP32-C3
    echo 2. The device should start in Bluetooth headset mode
    echo 3. Look for 'ESP32-C3 Headset' in your Bluetooth devices
    echo 4. Check the OLED display for status information
    echo.
    echo Troubleshooting:
    echo - If it doesn't work, try pressing the RESET button
    echo - Check serial monitor at 115200 baud for debug messages
    echo - Ensure all hardware connections are correct
    pause
)

endlocal
