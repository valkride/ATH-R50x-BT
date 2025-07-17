# Makefile for ESP32-C3 Bluetooth Headset Module

# Project configuration
PROJECT_NAME = bt_headset_module
IDF_TARGET = esp32c3
BUILD_DIR = build
DOCS_DIR = docs

# Default target
.PHONY: all
all: build

# Build commands
.PHONY: build
build:
	@echo "Building $(PROJECT_NAME)..."
	idf.py build

.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	idf.py clean

.PHONY: fullclean
fullclean:
	@echo "Full clean (including config)..."
	idf.py fullclean

.PHONY: reconfigure
reconfigure:
	@echo "Reconfiguring project..."
	idf.py reconfigure

# Flash commands
.PHONY: flash
flash:
	@echo "Flashing firmware..."
	idf.py flash

.PHONY: flash-monitor
flash-monitor:
	@echo "Flashing firmware and starting monitor..."
	idf.py flash monitor

.PHONY: monitor
monitor:
	@echo "Starting serial monitor..."
	idf.py monitor

.PHONY: erase
erase:
	@echo "Erasing flash..."
	idf.py erase-flash

# Configuration
.PHONY: menuconfig
menuconfig:
	@echo "Opening configuration menu..."
	idf.py menuconfig

.PHONY: save-defconfig
save-defconfig:
	@echo "Saving default configuration..."
	idf.py save-defconfig

# Development commands
.PHONY: size
size:
	@echo "Analyzing binary size..."
	idf.py size

.PHONY: size-components
size-components:
	@echo "Analyzing component sizes..."
	idf.py size-components

.PHONY: size-files
size-files:
	@echo "Analyzing file sizes..."
	idf.py size-files

# Testing and debugging
.PHONY: app-flash
app-flash:
	@echo "Flashing application only..."
	idf.py app-flash

.PHONY: bootloader-flash
bootloader-flash:
	@echo "Flashing bootloader only..."
	idf.py bootloader-flash

.PHONY: partition-table-flash
partition-table-flash:
	@echo "Flashing partition table only..."
	idf.py partition-table-flash

.PHONY: openocd
openocd:
	@echo "Starting OpenOCD for debugging..."
	idf.py openocd

.PHONY: gdb
gdb:
	@echo "Starting GDB debugger..."
	xtensa-esp32c3-elf-gdb $(BUILD_DIR)/$(PROJECT_NAME).elf

# Production commands
.PHONY: production-build
production-build:
	@echo "Building production version..."
	@echo "Setting optimization for size..."
	idf.py menuconfig
	idf.py build

.PHONY: factory-image
factory-image: build
	@echo "Creating factory image..."
	esptool.py --chip esp32c3 merge_bin -o $(BUILD_DIR)/factory.bin \
		0x0 $(BUILD_DIR)/bootloader/bootloader.bin \
		0x8000 $(BUILD_DIR)/partition_table/partition-table.bin \
		0x10000 $(BUILD_DIR)/$(PROJECT_NAME).bin

# Documentation
.PHONY: docs
docs:
	@echo "Available documentation:"
	@echo "  - README.md: Project overview"
	@echo "  - $(DOCS_DIR)/build_instructions.md: Build instructions"
	@echo "  - $(DOCS_DIR)/hardware_design.md: Hardware design guide"
	@echo "  - $(DOCS_DIR)/software_architecture.md: Software architecture"
	@echo "  - CHANGELOG.md: Version history"

# Quality assurance
.PHONY: check-format
check-format:
	@echo "Checking code format..."
	@find main -name "*.c" -o -name "*.h" | xargs clang-format -i
	@echo "Code formatting complete"

.PHONY: static-analysis
static-analysis:
	@echo "Running static analysis..."
	@if command -v cppcheck > /dev/null; then \
		cppcheck --enable=all --inconclusive --std=c99 main/; \
	else \
		echo "cppcheck not found, skipping static analysis"; \
	fi

# Utility commands
.PHONY: backup-config
backup-config:
	@echo "Backing up configuration..."
	cp sdkconfig sdkconfig.backup
	@echo "Configuration backed up to sdkconfig.backup"

.PHONY: restore-config
restore-config:
	@echo "Restoring configuration..."
	@if [ -f sdkconfig.backup ]; then \
		cp sdkconfig.backup sdkconfig; \
		echo "Configuration restored from sdkconfig.backup"; \
	else \
		echo "No backup configuration found"; \
	fi

.PHONY: reset-config
reset-config:
	@echo "Resetting to default configuration..."
	rm -f sdkconfig
	idf.py reconfigure

# Information
.PHONY: info
info:
	@echo "ESP32-C3 Bluetooth Headset Module"
	@echo "=================================="
	@echo "Project: $(PROJECT_NAME)"
	@echo "Target: $(IDF_TARGET)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo ""
	@echo "Available commands:"
	@echo "  make build         - Build the project"
	@echo "  make clean         - Clean build files"
	@echo "  make flash         - Flash firmware"
	@echo "  make monitor       - Start serial monitor"
	@echo "  make flash-monitor - Flash and start monitor"
	@echo "  make menuconfig    - Open configuration menu"
	@echo "  make size          - Show binary size"
	@echo "  make factory-image - Create factory image"
	@echo "  make docs          - Show documentation"
	@echo "  make info          - Show this information"

.PHONY: help
help: info

# Version information
.PHONY: version
version:
	@echo "Project version: 1.0.0"
	@echo "ESP-IDF version: $(shell idf.py --version 2>/dev/null || echo 'Not found')"
	@echo "Build date: $(shell date)"

# Default help target
.DEFAULT_GOAL := help
