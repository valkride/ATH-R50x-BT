/**
 * ESP32-C3 SuperMini Bluetooth Headset Firmware
 * 
 * Features:
 * - Split-cup design with left-cup ESP32-C3 SuperMini
 * - QCC5124 A2DP codec control
 * - TPA6120A2 headphone amplifier control
 * - SSD1306 OLED display (128x32)
 * - Four-button interface with debouncing
 * - Battery monitoring and charging status
 * - Voice Activity Detection (VAD)
 * - Basic noise suppression
 * - USB HID for Teams/Discord mute control
 * - P-MOSFET mic power control via flex cable
 * 
 * Hardware connections:
 * - Left cup: ESP32-C3, OLED, buttons, audio components
 * - Right cup: TP4056 charger, BLE mic module
 * - 6-wire flex cable: Vbat, GND, SDA, SCL, EN_MIC, STAT
 * 
 * Author: ESP32-C3 Audio Team
 * Date: July 2025
 * Version: 1.0.0
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <QCC5124Control.h>
#include <AudioProcessing.h>

// ====================================================================================
// PIN DEFINITIONS
// ====================================================================================

// I2C OLED Display
#define PIN_OLED_SDA    21  // GPIO21 - SDA
#define PIN_OLED_SCL    22  // GPIO22 - SCL

// ADC and Status
#define PIN_BAT_ADC     35  // GPIO35 - Battery voltage divider (ADC1_CH7)
#define PIN_STAT        34  // GPIO34 - TP4056 STAT line from right cup

// Power Control
#define PIN_EN_AUDIO    25  // GPIO25 - Enable QCC5124 + TPA6120A2 LDO
#define PIN_EN_MIC      26  // GPIO26 - Enable mic module in right cup (P-MOSFET gate)

// Buttons (active low with internal pullup)
#define PIN_BTN_PWR     27  // GPIO27 - Power button
#define PIN_BTN_VOL_UP  14  // GPIO14 - Volume up button
#define PIN_BTN_VOL_DN  12  // GPIO12 - Volume down button
#define PIN_BTN_MUTE    13  // GPIO13 - Mute button

// QCC5124 Control (UART or GPIO)
#define PIN_QCC_TX      10  // GPIO10 - UART TX to QCC5124
#define PIN_QCC_RX      9   // GPIO9 - UART RX from QCC5124
#define PIN_QCC_RST     2   // GPIO2 - QCC5124 Reset

// I2S Microphone Input (for VAD)
#define PIN_I2S_WS      18  // GPIO18 - I2S Word Select
#define PIN_I2S_SCK     19  // GPIO19 - I2S Serial Clock
#define PIN_I2S_SD      23  // GPIO23 - I2S Serial Data

// ====================================================================================
// CONSTANTS AND CONFIGURATION
// ====================================================================================

// OLED Display
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1
#define OLED_I2C_ADDR   0x3C

// Battery monitoring
#define BAT_VOLTAGE_DIVIDER 2.0f    // 2:1 voltage divider
#define BAT_FULL_VOLTAGE    4.2f    // Li-ion full voltage
#define BAT_EMPTY_VOLTAGE   3.0f    // Li-ion empty voltage
#define BAT_SAMPLES         32      // ADC averaging samples

// Button debouncing
#define BUTTON_DEBOUNCE_MS  50
#define BUTTON_LONG_PRESS_MS 1000

// Audio settings
#define AUDIO_SAMPLE_RATE   16000   // 16kHz for VAD
#define AUDIO_BUFFER_SIZE   512
#define VAD_THRESHOLD       0.02f   // Voice activity threshold
#define NOISE_GATE_ATTACK   5       // ms
#define NOISE_GATE_RELEASE  50      // ms

// System timing
#define DISPLAY_UPDATE_MS   100
#define BATTERY_CHECK_MS    1000
#define VAD_UPDATE_MS       20

// ====================================================================================
// GLOBAL VARIABLES
// ====================================================================================

// Hardware objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
USBHIDKeyboard keyboard;
QCC5124Control qccCodec(&Serial1);
AudioProcessor audioProcessor;
AudioEffects audioEffects;

// System state
bool systemPowered = false;
bool micEnabled = false;
bool audioEnabled = false;
bool micMuted = false;
bool vadActive = false;

// Battery monitoring
float batteryVoltage = 0.0f;
uint8_t batteryPercent = 0;
bool isCharging = false;
bool isChargingComplete = false;

// Button states
struct ButtonState {
    bool pressed;
    bool lastPressed;
    uint32_t pressTime;
    uint32_t releaseTime;
    bool longPressed;
};

ButtonState buttons[4] = {0}; // PWR, VOL_UP, VOL_DN, MUTE

// Audio buffers
int16_t audioBuffer[AUDIO_BUFFER_SIZE];
float vadBuffer[AUDIO_BUFFER_SIZE];
float noiseFloor = 0.0f;

// FreeRTOS handles
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t audioTaskHandle = NULL;
TaskHandle_t vadTaskHandle = NULL;
QueueHandle_t buttonQueue = NULL;
SemaphoreHandle_t i2cMutex = NULL;

// ADC calibration
esp_adc_cal_characteristics_t adcChars;

// ====================================================================================
// BUTTON EVENT TYPES
// ====================================================================================

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_PWR_SHORT,
    BTN_EVENT_PWR_LONG,
    BTN_EVENT_VOL_UP,
    BTN_EVENT_VOL_DOWN,
    BTN_EVENT_MUTE_TOGGLE,
    BTN_EVENT_MUTE_LONG
} button_event_t;

// ====================================================================================
// FORWARD DECLARATIONS
// ====================================================================================

void initializeHardware();
void initializeDisplay();
void initializeAudio();
void initializeBattery();
void initializeButtons();
void initializeUSB();

void updateDisplay();
void updateBattery();
void updateButtons();
void processButtonEvent(button_event_t event);

void enableAudio(bool enable);
void enableMic(bool enable);
void setMicMute(bool muted);
void sendVolumeCommand(bool up);
void sendTeamsMuteCommand();

void vadTask(void *parameter);
void displayTask(void *parameter);
void audioTask(void *parameter);

float calculateVAD(int16_t *samples, size_t length);
void applyNoiseGate(int16_t *samples, size_t length, float vadLevel);
void spectralSubtraction(int16_t *input, int16_t *output, size_t length);

// ====================================================================================
// MAIN SETUP AND LOOP
// ====================================================================================

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-C3 SuperMini Bluetooth Headset Starting...");
    
    // Initialize hardware subsystems
    initializeHardware();
    initializeDisplay();
    initializeAudio();
    initializeBattery();
    initializeButtons();
    initializeUSB();
    
    // Create FreeRTOS tasks
    xTaskCreate(displayTask, "DisplayTask", 4096, NULL, 1, &displayTaskHandle);
    xTaskCreate(audioTask, "AudioTask", 8192, NULL, 3, &audioTaskHandle);
    xTaskCreate(vadTask, "VADTask", 4096, NULL, 2, &vadTaskHandle);
    
    // Create synchronization objects
    buttonQueue = xQueueCreate(10, sizeof(button_event_t));
    i2cMutex = xSemaphoreCreateMutex();
    
    Serial.println("System initialized successfully");
    
    // Initial display update
    updateDisplay();
}

void loop() {
    // Handle button events
    button_event_t event;
    if (xQueueReceive(buttonQueue, &event, 0) == pdTRUE) {
        processButtonEvent(event);
    }
    
    // Update system state
    updateButtons();
    
    // Small delay for main loop
    vTaskDelay(pdMS_TO_TICKS(10));
}

// ====================================================================================
// HARDWARE INITIALIZATION
// ====================================================================================

void initializeHardware() {
    // Configure power control pins
    pinMode(PIN_EN_AUDIO, OUTPUT);
    pinMode(PIN_EN_MIC, OUTPUT);
    digitalWrite(PIN_EN_AUDIO, LOW);  // Audio off initially
    digitalWrite(PIN_EN_MIC, LOW);    // Mic off initially
    
    // Configure status input
    pinMode(PIN_STAT, INPUT_PULLUP);
    
    // Configure QCC5124 control
    pinMode(PIN_QCC_RST, OUTPUT);
    digitalWrite(PIN_QCC_RST, HIGH);  // Keep QCC5124 out of reset
    
    // Initialize I2C
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    Wire.setClock(400000); // 400kHz I2C
    
    Serial.println("Hardware initialized");
}

void initializeDisplay() {
    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        Serial.println("SSD1306 allocation failed");
        return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32-C3 Headset");
    display.println("Initializing...");
    display.display();
    
    Serial.println("Display initialized");
}

void initializeAudio() {
    // Initialize QCC5124 codec
    if (!qccCodec.begin()) {
        Serial.println("QCC5124 initialization failed!");
        return;
    }
    
    // Set codec callback for status updates
    qccCodec.setStatusCallback([](const String& status) {
        Serial.println("QCC5124 Status: " + status);
    });
    
    // Initialize audio processor
    if (!audioProcessor.begin()) {
        Serial.println("Audio processor initialization failed!");
        return;
    }
    
    // Initialize audio effects
    audioEffects.begin();
    audioEffects.enableAGC(true);
    audioEffects.setAGCParameters(0.5f, 0.001f, 0.01f);
    
    // Configure I2S for microphone input
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_SCK,
        .ws_io_num = PIN_I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_I2S_SD
    };
    
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_0);
    
    // Initialize UART for QCC5124 communication
    Serial1.begin(115200, SERIAL_8N1, PIN_QCC_RX, PIN_QCC_TX);
    
    Serial.println("Audio subsystem initialized");
}

void initializeBattery() {
    // Configure ADC for battery monitoring
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);  // GPIO35 = ADC1_CH7
    
    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adcChars);
    
    Serial.println("Battery monitoring initialized");
}

void initializeButtons() {
    // Configure button pins with internal pullup
    pinMode(PIN_BTN_PWR, INPUT_PULLUP);
    pinMode(PIN_BTN_VOL_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_VOL_DN, INPUT_PULLUP);
    pinMode(PIN_BTN_MUTE, INPUT_PULLUP);
    
    // Initialize button states
    for (int i = 0; i < 4; i++) {
        buttons[i].pressed = false;
        buttons[i].lastPressed = false;
        buttons[i].pressTime = 0;
        buttons[i].releaseTime = 0;
        buttons[i].longPressed = false;
    }
    
    Serial.println("Buttons initialized");
}

void initializeUSB() {
    // Initialize USB HID keyboard
    USB.begin();
    keyboard.begin();
    
    Serial.println("USB HID initialized");
}

// ====================================================================================
// DISPLAY TASK
// ====================================================================================

void displayTask(void *parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        // Update battery status
        updateBattery();
        
        // Update display
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            updateDisplay();
            xSemaphoreGive(i2cMutex);
        }
        
        // Wait for next update
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(DISPLAY_UPDATE_MS));
    }
}

void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    // Title
    display.println("ESP32-C3 Headset");
    
    // Battery status
    display.setCursor(0, 10);
    if (isChargingComplete) {
        display.print("Battery: Full");
    } else if (isCharging) {
        display.print("Charging: ");
        display.print(batteryPercent);
        display.print("%");
    } else {
        display.print("Battery: ");
        display.print(batteryPercent);
        display.print("%");
    }
    
    // System status
    display.setCursor(0, 20);
    display.print("Audio: ");
    display.print(audioEnabled ? "ON" : "OFF");
    display.print(" Vol: ");
    display.print(qccCodec.getVolume());
    
    // Connection and VAD status
    display.setCursor(0, 30);
    display.print("BT: ");
    display.print(qccCodec.getConnectionStatus() ? "CONN" : "DISC");
    display.print(" VAD: ");
    display.print(vadActive ? "ACT" : "SIL");
    display.print(" Mic: ");
    display.print(micEnabled && !micMuted ? "ON" : "OFF");
    
    display.display();
}

void updateBattery() {
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();
    
    if (now - lastUpdate < BATTERY_CHECK_MS) {
        return;
    }
    lastUpdate = now;
    
    // Read battery voltage (with averaging)
    uint32_t adcSum = 0;
    for (int i = 0; i < BAT_SAMPLES; i++) {
        adcSum += adc1_get_raw(ADC1_CHANNEL_7);  // GPIO35 = ADC1_CH7
        delayMicroseconds(100);
    }
    
    uint32_t adcReading = adcSum / BAT_SAMPLES;
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adcReading, &adcChars);
    batteryVoltage = (voltage / 1000.0f) * BAT_VOLTAGE_DIVIDER;
    
    // Calculate percentage
    batteryPercent = (uint8_t)((batteryVoltage - BAT_EMPTY_VOLTAGE) / 
                               (BAT_FULL_VOLTAGE - BAT_EMPTY_VOLTAGE) * 100);
    batteryPercent = constrain(batteryPercent, 0, 100);
    
    // Read charging status
    bool statPin = digitalRead(PIN_STAT);
    isCharging = !statPin;  // STAT is active low during charging
    isChargingComplete = statPin && (batteryPercent > 95);
}

// ====================================================================================
// BUTTON HANDLING
// ====================================================================================

void updateButtons() {
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();
    
    if (now - lastUpdate < 10) {  // Check every 10ms
        return;
    }
    lastUpdate = now;
    
    // Button pins in order: PWR, VOL_UP, VOL_DN, MUTE
    int buttonPins[] = {PIN_BTN_PWR, PIN_BTN_VOL_UP, PIN_BTN_VOL_DN, PIN_BTN_MUTE};
    button_event_t events[] = {BTN_EVENT_PWR_SHORT, BTN_EVENT_VOL_UP, BTN_EVENT_VOL_DOWN, BTN_EVENT_MUTE_TOGGLE};
    button_event_t longEvents[] = {BTN_EVENT_PWR_LONG, BTN_EVENT_NONE, BTN_EVENT_NONE, BTN_EVENT_MUTE_LONG};
    
    for (int i = 0; i < 4; i++) {
        bool currentState = !digitalRead(buttonPins[i]);  // Active low
        
        // Detect press
        if (currentState && !buttons[i].lastPressed) {
            buttons[i].pressTime = now;
            buttons[i].pressed = true;
            buttons[i].longPressed = false;
        }
        
        // Detect release
        if (!currentState && buttons[i].lastPressed) {
            buttons[i].releaseTime = now;
            uint32_t pressDuration = now - buttons[i].pressTime;
            
            if (pressDuration > BUTTON_DEBOUNCE_MS) {
                if (pressDuration > BUTTON_LONG_PRESS_MS && longEvents[i] != BTN_EVENT_NONE) {
                    xQueueSend(buttonQueue, &longEvents[i], 0);
                } else {
                    xQueueSend(buttonQueue, &events[i], 0);
                }
            }
            
            buttons[i].pressed = false;
        }
        
        // Check for long press while still pressed
        if (currentState && buttons[i].pressed && !buttons[i].longPressed) {
            if (now - buttons[i].pressTime > BUTTON_LONG_PRESS_MS) {
                buttons[i].longPressed = true;
                if (longEvents[i] != BTN_EVENT_NONE) {
                    xQueueSend(buttonQueue, &longEvents[i], 0);
                }
            }
        }
        
        buttons[i].lastPressed = currentState;
    }
}

void processButtonEvent(button_event_t event) {
    Serial.print("Button event: ");
    Serial.println(event);
    
    switch (event) {
        case BTN_EVENT_PWR_SHORT:
            // Toggle system power
            systemPowered = !systemPowered;
            enableAudio(systemPowered);
            Serial.println(systemPowered ? "System ON" : "System OFF");
            break;
            
        case BTN_EVENT_PWR_LONG:
            // Force shutdown
            systemPowered = false;
            enableAudio(false);
            enableMic(false);
            Serial.println("Force shutdown");
            break;
            
        case BTN_EVENT_VOL_UP:
            if (audioEnabled) {
                sendVolumeCommand(true);
                Serial.println("Volume UP");
            }
            break;
            
        case BTN_EVENT_VOL_DOWN:
            if (audioEnabled) {
                sendVolumeCommand(false);
                Serial.println("Volume DOWN");
            }
            break;
            
        case BTN_EVENT_MUTE_TOGGLE:
            // Toggle mic mute
            micMuted = !micMuted;
            setMicMute(micMuted);
            Serial.println(micMuted ? "Mic MUTED" : "Mic UNMUTED");
            break;
            
        case BTN_EVENT_MUTE_LONG:
            // Send Teams/Discord mute command
            sendTeamsMuteCommand();
            Serial.println("Teams/Discord mute command sent");
            break;
            
        default:
            break;
    }
}

// ====================================================================================
// AUDIO CONTROL
// ====================================================================================

void enableAudio(bool enable) {
    audioEnabled = enable;
    digitalWrite(PIN_EN_AUDIO, enable ? HIGH : LOW);
    
    if (enable) {
        // Enable QCC5124 and TPA6120A2
        delay(100);  // Power settling time
        
        // Initialize QCC5124 codec
        if (!qccCodec.isReady()) {
            Serial.println("QCC5124 not ready, attempting reset...");
            qccCodec.reset();
            delay(500);
        }
        
        // Start pairing mode
        qccCodec.startPairing();
        Serial.println("QCC5124 pairing mode enabled");
        
    } else {
        // Disconnect and disable audio components
        qccCodec.disconnect();
        enableMic(false);  // Also disable mic
    }
}

void enableMic(bool enable) {
    micEnabled = enable;
    digitalWrite(PIN_EN_MIC, enable ? HIGH : LOW);
}

void setMicMute(bool muted) {
    micMuted = muted;
    // Control mic enable based on mute state and VAD
    bool shouldEnableMic = micEnabled && !micMuted && vadActive;
    digitalWrite(PIN_EN_MIC, shouldEnableMic ? HIGH : LOW);
}

void sendVolumeCommand(bool up) {
    // Send volume command to QCC5124
    if (up) {
        qccCodec.volumeUp();
        Serial.println("Volume up sent to QCC5124");
    } else {
        qccCodec.volumeDown();
        Serial.println("Volume down sent to QCC5124");
    }
}

void sendTeamsMuteCommand() {
    // Send Ctrl+Shift+M to toggle mute in Teams/Discord
    keyboard.press(KEY_LEFT_CTRL);
    keyboard.press(KEY_LEFT_SHIFT);
    keyboard.press('m');
    delay(100);
    keyboard.releaseAll();
}

// ====================================================================================
// AUDIO PROCESSING TASK
// ====================================================================================

void audioTask(void *parameter) {
    size_t bytesRead;
    int16_t processedBuffer[AUDIO_BUFFER_SIZE];
    
    while (true) {
        // Process QCC5124 status updates
        qccCodec.processIncomingData();
        
        if (micEnabled && !micMuted) {
            // Read audio data from I2S
            i2s_read(I2S_NUM_0, audioBuffer, sizeof(audioBuffer), &bytesRead, portMAX_DELAY);
            
            if (bytesRead > 0) {
                size_t samples = bytesRead / sizeof(int16_t);
                
                // Apply advanced audio processing
                audioProcessor.processFrame(audioBuffer, processedBuffer, samples);
                
                // Apply audio effects
                float floatBuffer[AUDIO_BUFFER_SIZE];
                float outputBuffer[AUDIO_BUFFER_SIZE];
                
                // Convert to float for effects processing
                for (size_t i = 0; i < samples; i++) {
                    floatBuffer[i] = (float)processedBuffer[i] / 32768.0f;
                }
                
                // Apply effects
                audioEffects.processFrame(floatBuffer, outputBuffer, samples);
                
                // Convert back to int16
                for (size_t i = 0; i < samples; i++) {
                    processedBuffer[i] = (int16_t)(outputBuffer[i] * 32768.0f);
                }
                
                // Update VAD state
                vadActive = audioProcessor.isVoiceActive();
                
                // Update mic enable based on VAD
                bool shouldEnableMic = micEnabled && !micMuted && vadActive;
                if (shouldEnableMic != digitalRead(PIN_EN_MIC)) {
                    digitalWrite(PIN_EN_MIC, shouldEnableMic ? HIGH : LOW);
                }
                
                // Optional: Send processed audio somewhere (e.g., Bluetooth transmission)
                // This would be implemented based on your specific audio routing needs
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(VAD_UPDATE_MS));
    }
}

// ====================================================================================
// VAD TASK
// ====================================================================================

void vadTask(void *parameter) {
    while (true) {
        // VAD processing is now handled in audioTask
        // This task can be used for additional audio processing
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ====================================================================================
// AUDIO PROCESSING FUNCTIONS
// ====================================================================================

float calculateVAD(int16_t *samples, size_t length) {
    float energy = 0.0f;
    float zeroCrossings = 0.0f;
    
    // Calculate energy
    for (size_t i = 0; i < length; i++) {
        float sample = (float)samples[i] / 32768.0f;
        energy += sample * sample;
    }
    energy = sqrt(energy / length);
    
    // Calculate zero crossings
    for (size_t i = 1; i < length; i++) {
        if ((samples[i] > 0 && samples[i-1] < 0) || (samples[i] < 0 && samples[i-1] > 0)) {
            zeroCrossings++;
        }
    }
    zeroCrossings /= length;
    
    // Simple VAD decision based on energy and zero crossings
    float vadScore = energy * 0.7f + zeroCrossings * 0.3f;
    
    // Update noise floor (exponential smoothing)
    static float noiseFloorEMA = 0.0f;
    noiseFloorEMA = 0.95f * noiseFloorEMA + 0.05f * energy;
    noiseFloor = noiseFloorEMA;
    
    return vadScore;
}

void applyNoiseGate(int16_t *samples, size_t length, float vadLevel) {
    static float gateLevel = 0.0f;
    static bool gateOpen = false;
    
    // Gate decision
    if (vadLevel > VAD_THRESHOLD && !gateOpen) {
        gateOpen = true;
    } else if (vadLevel < VAD_THRESHOLD * 0.7f && gateOpen) {
        gateOpen = false;
    }
    
    // Apply gate with attack/release
    float targetLevel = gateOpen ? 1.0f : 0.0f;
    float attackRelease = gateOpen ? 
        (1.0f - exp(-1.0f / (NOISE_GATE_ATTACK * AUDIO_SAMPLE_RATE / 1000.0f))) :
        (1.0f - exp(-1.0f / (NOISE_GATE_RELEASE * AUDIO_SAMPLE_RATE / 1000.0f)));
    
    for (size_t i = 0; i < length; i++) {
        gateLevel += attackRelease * (targetLevel - gateLevel);
        samples[i] = (int16_t)(samples[i] * gateLevel);
    }
}

void spectralSubtraction(int16_t *input, int16_t *output, size_t length) {
    // Simple spectral subtraction (placeholder implementation)
    // In practice, this would use FFT/IFFT for frequency domain processing
    
    static float alpha = 0.95f;  // Smoothing factor
    static float noiseEst[AUDIO_BUFFER_SIZE] = {0};
    
    for (size_t i = 0; i < length; i++) {
        float sample = (float)input[i];
        
        // Update noise estimate
        noiseEst[i] = alpha * noiseEst[i] + (1.0f - alpha) * abs(sample);
        
        // Apply spectral subtraction
        float cleanSample = sample - noiseEst[i] * 0.5f;
        
        // Apply floor to prevent over-subtraction
        if (abs(cleanSample) < abs(sample) * 0.1f) {
            cleanSample = sample * 0.1f;
        }
        
        output[i] = (int16_t)constrain(cleanSample, -32768, 32767);
    }
}
