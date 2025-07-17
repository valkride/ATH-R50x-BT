# Hardware Design Guide

## PCB Layout Considerations

### Power Supply
- **Input**: 3.7V Li-ion battery or 5V USB-C
- **Regulation**: 3.3V LDO regulator (AMS1117-3.3 or similar)
- **Battery Management**: TP4056 charging IC with protection
- **Power Switch**: Optional tactile switch for hard power off

### ESP32-C3 Module
- **Module**: ESP32-C3-WROOM-02 (4MB Flash, integrated antenna)
- **Crystal**: 40MHz (usually integrated in module)
- **Reset Circuit**: 10kΩ pullup on EN pin, 0.1μF capacitor to GND
- **Boot Circuit**: 10kΩ pullup on GPIO9, button to GND for programming

### Audio Circuit
- **DAC**: PCM5102A I2S DAC module
- **Connections**:
  - VCC → 3.3V
  - GND → GND
  - BCK → GPIO4 (I2S Bit Clock)
  - LCK → GPIO5 (I2S Word Select)
  - DIN → GPIO6 (I2S Data)
  - SCK → GND (System Clock tied low)
  - FLT → 3.3V (Filter select)
  - FMT → GND (I2S format)
  - XMT → 3.3V (Soft mute control)

### Audio Output
- **Output**: 3.5mm stereo jack or direct to headphone drivers
- **Coupling**: 470μF capacitors on L/R audio outputs
- **Filtering**: 47pF capacitors to GND on audio outputs

### User Interface
- **Buttons**: 3x tactile switches (6x6mm)
  - Play/Pause → GPIO0
  - Volume Up → GPIO1
  - Volume Down → GPIO2
  - All buttons: 10kΩ pullup resistors
- **LED**: Single RGB LED or 3x individual LEDs
  - Status LED → GPIO8
  - Current limiting resistor: 330Ω

### Battery Monitoring
- **Voltage Divider**: 2:1 ratio for Li-ion monitoring
  - R1: 10kΩ (to battery positive)
  - R2: 10kΩ (to GND)
  - Junction → GPIO3 (ADC1_CH3)

### Programming Interface
- **USB-C**: For charging and programming
- **UART**: Built into ESP32-C3 module
- **Boot Button**: Connected to GPIO9 for programming mode

## Component List

### Main Components
- ESP32-C3-WROOM-02 module
- PCM5102A DAC module
- TP4056 charging module
- AMS1117-3.3V regulator
- 3.7V Li-ion battery (500-1000mAh)
- USB-C connector

### Passive Components
- Resistors: 10kΩ (x6), 330Ω (x1)
- Capacitors: 0.1μF (x3), 470μF (x2), 47pF (x2)
- Inductors: 10μH (x1) for power filtering

### Mechanical
- 3x 6x6mm tactile switches
- 1x 3.5mm stereo jack
- 1x 5mm LED (or RGB LED)
- PCB: 2-layer, 1.6mm thickness

## Assembly Instructions

### Step 1: Power Circuit
1. Solder TP4056 charging module
2. Connect battery connector with protection
3. Install voltage regulator with filtering capacitors
4. Add power switch (optional)

### Step 2: ESP32-C3 Module
1. Solder ESP32-C3 module to PCB
2. Add reset circuit components
3. Connect programming interface
4. Install boot button

### Step 3: Audio Circuit
1. Solder PCM5102A DAC module
2. Connect I2S signals to ESP32-C3
3. Add audio output coupling capacitors
4. Connect 3.5mm jack

### Step 4: User Interface
1. Install tactile switches with pullup resistors
2. Mount status LED with current limiting resistor
3. Connect battery monitoring circuit

### Step 5: Testing
1. Program ESP32-C3 with firmware
2. Test all buttons and LED functionality
3. Verify audio output quality
4. Check battery monitoring
5. Test Bluetooth connectivity

## PCB Design Rules

### Trace Widths
- Power traces: 0.5mm minimum
- Signal traces: 0.2mm minimum
- I2S traces: 0.3mm, keep short and matched length

### Via Sizes
- Standard vias: 0.2mm drill, 0.45mm pad
- Power vias: 0.3mm drill, 0.6mm pad

### Clearances
- Trace to trace: 0.15mm minimum
- Trace to pad: 0.1mm minimum
- Via to via: 0.2mm minimum

### Ground Plane
- Use solid ground plane on bottom layer
- Ground vias under ESP32-C3 module
- Separate analog and digital ground areas

## Manufacturing Notes

### PCB Specifications
- Layers: 2
- Thickness: 1.6mm
- Copper weight: 1oz (35μm)
- Surface finish: HASL or ENIG
- Solder mask: Green
- Silkscreen: White

### Assembly Process
1. Apply solder paste using stencil
2. Place components with pick-and-place
3. Reflow solder in oven
4. Wave solder through-hole components
5. Clean flux residue
6. Functional testing

### Quality Control
- Visual inspection for solder joints
- Continuity testing of all nets
- Power supply verification
- Audio quality testing
- Bluetooth range testing
- Battery life testing

## Housing Integration

### Mechanical Considerations
- PCB dimensions: 40mm x 30mm maximum
- Component height: 8mm maximum
- Mounting holes: 2.5mm diameter
- Button accessibility
- LED visibility
- Audio jack accessibility
- USB-C port accessibility

### Environmental Protection
- Conformal coating for moisture protection
- ESD protection on exposed connectors
- Strain relief for battery wires
- Proper grounding for EMI compliance

## Safety Considerations

### Electrical Safety
- Proper battery protection circuit
- Overcurrent protection
- Thermal protection
- Isolation between charging and system circuits

### EMC Compliance
- Proper grounding techniques
- Filtered power supplies
- Shielded cables where necessary
- Antenna placement away from sensitive circuits

### Mechanical Safety
- Smooth edges on PCB
- Secure mounting of heavy components
- No sharp points or edges
- Proper stress relief for connectors
