# ESP32-C3 SuperMini Bluetooth Headset

## Quick Links
- [📖 Documentation](https://github.com/yourusername/esp32-c3-bluetooth-headset/wiki)
- [🐛 Report Bug](https://github.com/yourusername/esp32-c3-bluetooth-headset/issues/new?template=bug_report.md)
- [💡 Request Feature](https://github.com/yourusername/esp32-c3-bluetooth-headset/issues/new?template=feature_request.md)
- [💬 Discussions](https://github.com/yourusername/esp32-c3-bluetooth-headset/discussions)

## What's This?
A complete Arduino-style C++ firmware for ESP32-C3 SuperMini based split-cup Bluetooth headset with advanced audio processing.

## Key Features
- 🎧 Split-cup design with single ESP32-C3 control
- 🔊 QCC5124 A2DP codec with I2C control
- 🎙️ Voice Activity Detection and noise suppression
- 📱 USB HID for Teams/Discord mute
- 🔋 Smart battery management
- 📺 OLED status display

## Hardware
- **ESP32-C3 SuperMini** (left cup)
- **QCC5124** Bluetooth codec
- **SSD1306** OLED display
- **TP4056** battery charger (right cup)
- **6-wire flex cable** connection

## Get Started
```bash
git clone https://github.com/yourusername/esp32-c3-bluetooth-headset.git
cd esp32-c3-bluetooth-headset
pio run --target upload
```

## Community
- **Issues**: Bug reports and feature requests
- **Discussions**: Questions and community chat
- **Wiki**: Detailed documentation
- **Contributing**: See CONTRIBUTING.md

## License
MIT License - see LICENSE file

---
Made with ❤️ for the ESP32 community
