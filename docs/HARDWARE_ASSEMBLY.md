# Hardware Assembly Guide

This guide provides detailed instructions for assembling the ESP32-C3 SuperMini Bluetooth Headset hardware.

## 📋 Bill of Materials (BOM)

### Left Earcup Components
| Component | Part Number | Description | Quantity |
|-----------|-------------|-------------|----------|
| ESP32-C3 SuperMini | ESP32-C3-WROOM-02 | Main microcontroller | 1 |
| SSD1306 OLED | 0.96" 128x32 I2C | Status display | 1 |
| QCC5124 | QCC5124 A2DP Module | Bluetooth codec | 1 |
| TPA6120A2 | TPA6120A2 | Headphone amplifier | 1 |
| TPS7A4700 | TPS7A4700 | LDO regulator | 1 |
| Push Buttons | 6x6mm tactile | User interface | 4 |
| Resistors | 10kΩ 0603 | Pull-up resistors | 4 |
| Capacitors | 10µF, 100nF | Power filtering | Various |

### Right Earcup Components
| Component | Part Number | Description | Quantity |
|-----------|-------------|-------------|----------|
| TP4056 | TP4056 Module | Battery charger | 1 |
| BLE Mic Module | Generic BLE mic | Microphone | 1 |
| P-MOSFET | AO3401 | Power switch | 1 |
| Li-ion Battery | 18650 or 21700 | Power source | 1 |
| USB-C Connector | USB-C receptacle | Charging port | 1 |

### Interconnection
| Component | Description | Quantity |
|-----------|-------------|----------|
| Flex Cable | 6-wire 0.5mm pitch | Inter-cup connection | 1 |
| Connectors | JST-PH 2.0mm | Cable connectors | 2 |

## 🔧 Assembly Instructions

### Left Earcup Assembly

#### Step 1: ESP32-C3 SuperMini Preparation

1. **Verify board functionality:**
   - Connect to USB-C
   - Check power LED
   - Test with simple blink sketch

2. **Prepare GPIO pins:**
   - Solder header pins if needed
   - Test continuity to ensure good connections

#### Step 2: Power Supply Circuit

1. **Install TPS7A4700 LDO:**
   ```
   VIN (5V) → TPS7A4700 → VOUT (3.3V)
   GND → GND
   EN → GPIO25 (Audio enable)
   ```

2. **Add power filtering:**
   - 10µF capacitor on input
   - 100nF ceramic capacitor on output
   - Connect to ESP32-C3 VCC and GND

#### Step 3: I2C Display Connection

1. **SSD1306 OLED wiring:**
   ```
   ESP32-C3 Pin 21 (GPIO21) → SDA
   ESP32-C3 Pin 22 (GPIO22) → SCL
   3.3V → VCC
   GND → GND
   ```

2. **I2C pull-up resistors:**
   - 4.7kΩ resistor from SDA to 3.3V
   - 4.7kΩ resistor from SCL to 3.3V

#### Step 4: QCC5124 Connection

1. **I2C interface:**
   ```
   ESP32-C3 Pin 21 (GPIO21) → SDA
   ESP32-C3 Pin 22 (GPIO22) → SCL
   ESP32-C3 Pin 2 (GPIO2) → RESET
   3.3V → VCC
   GND → GND
   ```

2. **Power control:**
   - Connect QCC5124 VCC to TPS7A4700 output
   - Ensure proper power sequencing

#### Step 5: Button Interface

1. **Button connections:**
   ```
   GPIO27 → Power button → GND
   GPIO14 → Volume up button → GND
   GPIO12 → Volume down button → GND
   GPIO13 → Mute button → GND
   ```

2. **Pull-up resistors:**
   - 10kΩ resistor from each GPIO to 3.3V
   - Or use internal pull-ups (configured in software)

#### Step 6: Audio Processing

1. **TPA6120A2 connections:**
   ```
   Audio Input → QCC5124 audio output
   Power → TPS7A4700 output
   Output → Headphone drivers
   ```

2. **I2S microphone interface:**
   ```
   GPIO18 → I2S_WS (Word Select)
   GPIO19 → I2S_SCK (Serial Clock)
   GPIO23 → I2S_SD (Serial Data)
   ```

### Right Earcup Assembly

#### Step 1: Battery and Charging Circuit

1. **TP4056 module:**
   - Connect Li-ion battery to BAT+ and BAT-
   - Connect USB-C to VCC and GND
   - STAT output to flex cable

2. **Power distribution:**
   - Battery positive to flex cable VBat
   - Battery negative to flex cable GND

#### Step 2: Microphone Module

1. **BLE mic module:**
   - VCC → P-MOSFET drain
   - GND → Flex cable GND
   - Data lines to flex cable (if needed)

2. **P-MOSFET control:**
   ```
   Gate → Flex cable EN_MIC
   Source → Flex cable VBat
   Drain → Mic module VCC
   ```

### Flex Cable Assembly

#### Step 1: Cable Preparation

1. **Wire assignments:**
   ```
   Wire 1: VBat (Red)
   Wire 2: GND (Black)
   Wire 3: SDA (Blue)
   Wire 4: SCL (White)
   Wire 5: EN_MIC (Green)
   Wire 6: STAT (Yellow)
   ```

2. **Connector attachment:**
   - Crimp JST-PH connectors on both ends
   - Use strain relief for durability

#### Step 2: Testing

1. **Continuity test:**
   - Check all 6 wires for continuity
   - Ensure no shorts between wires

2. **Voltage test:**
   - VBat should read battery voltage
   - GND should be stable reference

## 🔍 Testing and Validation

### Power-On Testing

1. **Initial power check:**
   - Connect battery
   - Check 3.3V regulation
   - Verify ESP32-C3 boots

2. **Display test:**
   - OLED should show startup screen
   - Check I2C communication

3. **Button test:**
   - Each button should register presses
   - Check debouncing and long-press

### Audio System Testing

1. **QCC5124 communication:**
   - Check I2C address detection
   - Verify codec initialization
   - Test volume control

2. **Audio path:**
   - Pair with Bluetooth device
   - Test audio playback
   - Check amplifier output

### Battery System Testing

1. **Charging test:**
   - Connect USB-C charger
   - Check charging LED
   - Verify STAT line operation

2. **Battery monitoring:**
   - Check voltage reading accuracy
   - Test low battery detection

## 📐 Mechanical Assembly

### Earcup Integration

1. **Component placement:**
   - Route flex cable through headband
   - Secure components with adhesive
   - Ensure button accessibility

2. **Strain relief:**
   - Use grommets for cable entry
   - Secure internal connections
   - Test mechanical durability

### Final Assembly

1. **Housing closure:**
   - Ensure all components fit
   - Check for short circuits
   - Secure all connections

2. **Testing:**
   - Complete system test
   - Check all functions
   - Verify audio quality

## 🛠️ Troubleshooting

### Common Issues

**No power:**
- Check battery voltage
- Verify LDO enable signal
- Check power connections

**No display:**
- Verify I2C connections
- Check pull-up resistors
- Test OLED module separately

**No audio:**
- Check QCC5124 power
- Verify I2C communication
- Test amplifier separately

**Charging issues:**
- Check USB-C connection
- Verify TP4056 operation
- Test battery connection

### Debug Tools

- **Multimeter**: Voltage and continuity testing
- **Oscilloscope**: I2C and audio signal analysis
- **Logic analyzer**: Digital signal debugging
- **Audio analyzer**: Frequency response testing

## 📚 References

- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [QCC5124 Reference Manual](https://www.qualcomm.com/products/qcc5124)
- [TPA6120A2 Datasheet](https://www.ti.com/lit/ds/symlink/tpa6120a2.pdf)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

## 🔧 Tools Required

- Soldering iron and solder
- Multimeter
- Wire strippers
- Heat shrink tubing
- Flux and desoldering wick
- Small screwdrivers
- Crimping tool for connectors

---

**Safety Note**: Always work with proper ESD protection and in a well-ventilated area when soldering.
