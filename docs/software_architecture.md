# Software Architecture

## Overview

The ESP32-C3 Bluetooth headset module firmware is designed with a modular architecture that separates concerns and provides clean interfaces between components. The system uses FreeRTOS for task management and inter-task communication.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Main Application                        │
│                   (main.c)                                 │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────┴───────────────────────────────────┐
│                 System Event Queue                         │
│              (Inter-module Communication)                  │
└─────────┬───────────┬───────────┬───────────┬───────────────┘
          │           │           │           │
    ┌─────▼─────┐ ┌───▼─────┐ ┌───▼─────┐ ┌───▼─────┐
    │Bluetooth  │ │ Audio   │ │ Power   │ │   UI    │
    │ Manager   │ │ Manager │ │ Manager │ │ Manager │
    └─────┬─────┘ └───┬─────┘ └───┬─────┘ └───┬─────┘
          │           │           │           │
    ┌─────▼─────┐ ┌───▼─────┐ ┌───▼─────┐ ┌───▼─────┐
    │ESP-IDF BT │ │ I2S     │ │ ADC     │ │ GPIO    │
    │  Stack    │ │ Driver  │ │ Driver  │ │ Driver  │
    └───────────┘ └─────────┘ └─────────┘ └─────────┘
```

## Core Components

### 1. Main Application (main.c)

**Purpose**: System initialization and main event loop
**Key Functions**:
- System startup and initialization
- Event coordination between modules
- Error handling and recovery
- System state management

**Main Tasks**:
- `system_main_task()`: Main event processing loop
- Event routing and system state machine
- Health monitoring and error recovery

### 2. Bluetooth Manager (bluetooth_manager.c)

**Purpose**: Bluetooth stack management and wireless communication
**Key Functions**:
- Bluetooth stack initialization
- Device pairing and connection management
- A2DP sink for audio streaming
- AVRC controller for media control
- Device discovery and reconnection

**Bluetooth Profiles**:
- **A2DP Sink**: Receives audio stream from source device
- **AVRC Controller**: Sends media control commands
- **HFP**: Hands-free profile for calls (future enhancement)

**State Machine**:
```
IDLE → DISCOVERABLE → CONNECTING → CONNECTED → PLAYING
  ↑                                     ↓
  └─────────────────────────────────────┘
```

### 3. Audio Manager (audio_manager.c)

**Purpose**: Audio processing and I2S output
**Key Functions**:
- I2S driver configuration and management
- Audio buffer management
- Volume control and processing
- Audio format handling
- Real-time audio streaming

**Audio Pipeline**:
```
Bluetooth → Ring Buffer → Volume Control → I2S → Audio Output
```

**Features**:
- Configurable sample rates (44.1kHz, 48kHz)
- 16-bit audio processing
- Logarithmic volume control
- Audio fade effects
- Buffer underrun protection

### 4. Power Manager (power_manager.c)

**Purpose**: Power management and battery monitoring
**Key Functions**:
- Battery voltage monitoring
- Power state management
- Sleep mode control
- Activity timeout handling
- Low battery warnings

**Power States**:
- **ACTIVE**: Normal operation
- **IDLE**: Ready for sleep
- **SLEEP**: Light sleep mode
- **DEEP_SLEEP**: Deep sleep mode
- **SHUTDOWN**: System shutdown

**Battery Management**:
- Voltage monitoring via ADC
- Battery percentage calculation
- Charging status detection
- Low battery warnings
- Automatic shutdown protection

### 5. UI Manager (ui_manager.c)

**Purpose**: User interface handling
**Key Functions**:
- Button input processing
- LED status indication
- Touch event handling
- Visual feedback control
- User interaction management

**Button Features**:
- Debounce filtering
- Long press detection
- Double-click recognition
- Multi-button combinations

**LED Patterns**:
- Solid: Connected
- Slow blink: Discoverable
- Fast blink: Pairing or low battery
- Breathing: Charging
- Off: Powered off

## Inter-Module Communication

### Event System

The system uses a centralized event queue for inter-module communication:

```c
typedef struct {
    system_event_type_t type;
    void *data;
} system_event_t;
```

**Event Types**:
- `SYSTEM_EVENT_BT_CONNECTED`: Bluetooth connection established
- `SYSTEM_EVENT_BT_DISCONNECTED`: Bluetooth disconnected
- `SYSTEM_EVENT_AUDIO_PLAY`: Audio playback started
- `SYSTEM_EVENT_AUDIO_PAUSE`: Audio playback paused
- `SYSTEM_EVENT_BUTTON_*`: Button press events
- `SYSTEM_EVENT_POWER_*`: Power management events

### Data Flow

1. **Audio Data Path**:
   ```
   Bluetooth A2DP → Audio Callback → Ring Buffer → I2S Task → Hardware
   ```

2. **Control Path**:
   ```
   Button Press → UI Manager → Event Queue → Bluetooth Manager → Remote Device
   ```

3. **Status Path**:
   ```
   System State → Event Queue → UI Manager → LED Display
   ```

## Configuration System

### Compile-Time Configuration

Using ESP-IDF's menuconfig system and Kconfig files:

- `Kconfig.projbuild`: Project-specific configuration options
- `sdkconfig.defaults`: Default configuration values
- `config.h`: C header with configuration constants

### Runtime Configuration

- NVS (Non-Volatile Storage) for paired device information
- Dynamic audio format configuration
- User preference storage

## Memory Management

### Static Allocation

- Fixed-size buffers for audio data
- Statically allocated task stacks
- Compile-time configured buffer sizes

### Dynamic Allocation

- Bluetooth stack memory
- Audio codec buffers
- Event queue entries

### Memory Optimization

- Shared buffers where possible
- Efficient data structures
- Memory pooling for frequent allocations

## Error Handling

### Error Categories

1. **Hardware Errors**: I2S, ADC, GPIO failures
2. **Bluetooth Errors**: Connection failures, stack errors
3. **System Errors**: Memory allocation, task creation failures
4. **Audio Errors**: Buffer underruns, format errors

### Error Recovery

- Automatic retry mechanisms
- Graceful degradation
- System restart as last resort
- Error logging and reporting

### Watchdog Protection

- Task watchdog for critical tasks
- Interrupt watchdog for system responsiveness
- Hardware watchdog for system failures

## Performance Considerations

### Real-Time Requirements

- Audio processing: < 10ms latency
- Button response: < 50ms
- Bluetooth events: < 100ms

### CPU Usage Optimization

- Efficient audio processing algorithms
- Minimal interrupt service routines
- Optimized task priorities
- DMA usage for data transfers

### Memory Usage

- Audio buffers: 4KB ring buffer
- Stack usage: Monitored and optimized
- Heap usage: Minimized dynamic allocation

## Testing Strategy

### Unit Testing

- Individual module testing
- Mock interfaces for hardware
- Automated test execution

### Integration Testing

- Module interaction testing
- End-to-end scenarios
- Performance validation

### Hardware Testing

- Audio quality measurements
- Battery life testing
- Bluetooth range testing
- Environmental testing

## Future Enhancements

### Planned Features

1. **Codec Support**: AAC, aptX codec support
2. **Multi-Device**: Multiple device pairing
3. **Voice Assistant**: Voice command integration
4. **Firmware Updates**: Over-the-air updates
5. **Advanced Audio**: EQ, noise cancellation

### Architecture Improvements

1. **Plugin System**: Modular audio processing
2. **State Persistence**: Enhanced NVS usage
3. **Performance Monitoring**: Real-time metrics
4. **Error Analytics**: Error pattern analysis

## Development Guidelines

### Code Style

- Follow ESP-IDF coding standards
- Use consistent naming conventions
- Document all public functions
- Include error handling in all functions

### Version Control

- Feature branches for development
- Code reviews for all changes
- Semantic versioning
- Change log maintenance

### Documentation

- API documentation
- Architecture documentation
- User manuals
- Hardware integration guides
