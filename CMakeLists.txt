# CMakeLists.txt for ESP32-C3 Bluetooth Headset Module

cmake_minimum_required(VERSION 3.16)

# Set the project name and version
project(bt_headset_module VERSION 1.0.0)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set target chip
set(IDF_TARGET esp32c3)

# Set component directories
set(EXTRA_COMPONENT_DIRS "components")

# Configure project
project(bt_headset_module)
