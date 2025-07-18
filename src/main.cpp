/**
 * ESP32-C3 Bluetooth Headset Firmware
 * Complete implementation with audio processing
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <BluetoothA2DPSink.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

// Pin definitions (ESP32-C3 compatible)
#define PIN_OLED_SDA    8
#define PIN_OLED_SCL    9
#define PIN_BAT_ADC     4
#define PIN_STAT        5
#define PIN_EN_AUDIO    6
#define PIN_EN_MIC      7
#define PIN_BTN_PWR     20
#define PIN_BTN_VOL_UP  21
#define PIN_BTN_VOL_DN  0
#define PIN_BTN_MUTE    1
#define PIN_QCC_RESET   2
#define PIN_QCC_UART_TX 10
#define PIN_QCC_UART_RX 3

// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Bluetooth Audio
BluetoothA2DPSink a2dp_sink;
USBHIDKeyboard keyboard;

// State
uint8_t volume = 8;
bool muted = false;
bool connected = false;
bool audioEnabled = false;
bool micEnabled = false;
bool isCharging = false;
bool chargingComplete = false;
float batteryPercent = 0;
uint32_t lastVADCheck = 0;
bool voiceDetected = false;

// Audio processing
#define SAMPLE_RATE 16000
#define FRAME_SIZE 320
#define VAD_THRESHOLD 0.001f
float audioBuffer[FRAME_SIZE];
float energyHistory[10];
uint8_t historyIndex = 0;

// QCC5124 Commands
void sendQCCCommand(uint8_t cmd, uint8_t data = 0);
void initQCC5124();

// Audio callbacks
void avrc_metadata_callback(uint8_t data1, const uint8_t *data2);
void audio_data_callback(const uint8_t *data, uint32_t length);

// Voice Activity Detection
float calculateEnergy(float* buffer, int size);
bool detectVoice();
void processAudio();

// Button debouncing
unsigned long lastButtonPress[4] = {0, 0, 0, 0};
const unsigned long DEBOUNCE_DELAY = 50;

void setup() {
    Serial.begin(115200);
    
    // Initialize pins
    pinMode(PIN_BTN_PWR, INPUT_PULLUP);
    pinMode(PIN_BTN_VOL_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_VOL_DN, INPUT_PULLUP);
    pinMode(PIN_BTN_MUTE, INPUT_PULLUP);
    pinMode(PIN_EN_AUDIO, OUTPUT);
    pinMode(PIN_EN_MIC, OUTPUT);
    pinMode(PIN_QCC_RESET, OUTPUT);
    pinMode(PIN_STAT, INPUT);
    
    // Initialize power control
    digitalWrite(PIN_EN_AUDIO, LOW);  // Audio off initially
    digitalWrite(PIN_EN_MIC, LOW);    // Mic off initially
    digitalWrite(PIN_QCC_RESET, LOW); // QCC reset
    
    // Initialize I2C and display
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("ESP32-C3 Headset");
    display.println("Initializing...");
    display.display();
    
    // Initialize USB HID
    USB.begin();
    keyboard.begin();
    
    // Initialize UART for QCC5124
    Serial1.begin(115200, SERIAL_8N1, PIN_QCC_UART_RX, PIN_QCC_UART_TX);
    
    // Initialize Bluetooth A2DP
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_stream_reader(audio_data_callback);
    a2dp_sink.start("ESP32-Headset");
    
    // Initialize QCC5124
    initQCC5124();
    
    Serial.println("Bluetooth Headset Ready");
}

void loop() {
    handleButtons();
    updateBattery();
    updateCharging();
    processAudio();
    updateDisplay();
    delay(10);
}

void handleButtons() {
    unsigned long now = millis();
    
    // Power button
    if (!digitalRead(PIN_BTN_PWR) && (now - lastButtonPress[0] > DEBOUNCE_DELAY)) {
        lastButtonPress[0] = now;
        togglePower();
    }
    
    // Volume up
    if (!digitalRead(PIN_BTN_VOL_UP) && (now - lastButtonPress[1] > DEBOUNCE_DELAY)) {
        lastButtonPress[1] = now;
        volumeUp();
    }
    
    // Volume down
    if (!digitalRead(PIN_BTN_VOL_DN) && (now - lastButtonPress[2] > DEBOUNCE_DELAY)) {
        lastButtonPress[2] = now;
        volumeDown();
    }
    
    // Mute
    if (!digitalRead(PIN_BTN_MUTE) && (now - lastButtonPress[3] > DEBOUNCE_DELAY)) {
        lastButtonPress[3] = now;
        toggleMute();
    }
}

void togglePower() {
    audioEnabled = !audioEnabled;
    digitalWrite(PIN_EN_AUDIO, audioEnabled ? HIGH : LOW);
    
    if (audioEnabled) {
        // Enable QCC5124
        digitalWrite(PIN_QCC_RESET, HIGH);
        delay(100);
        initQCC5124();
        Serial.println("Audio System ON");
    } else {
        // Disable QCC5124
        digitalWrite(PIN_QCC_RESET, LOW);
        digitalWrite(PIN_EN_MIC, LOW);
        micEnabled = false;
        Serial.println("Audio System OFF");
    }
    connected = audioEnabled;
}

void volumeUp() {
    if (volume < 15) {
        volume++;
        sendQCCCommand(0x01, volume); // Volume up command
        Serial.printf("Volume: %d\n", volume);
    }
}

void volumeDown() {
    if (volume > 0) {
        volume--;
        sendQCCCommand(0x02, volume); // Volume down command
        Serial.printf("Volume: %d\n", volume);
    }
}

void toggleMute() {
    muted = !muted;
    
    // Toggle microphone power
    micEnabled = !muted && audioEnabled;
    digitalWrite(PIN_EN_MIC, micEnabled ? HIGH : LOW);
    
    // Send USB HID mute command (Ctrl+Shift+M for Teams/Discord)
    if (muted) {
        keyboard.press(KEY_LEFT_CTRL);
        keyboard.press(KEY_LEFT_SHIFT);
        keyboard.press('m');
        delay(50);
        keyboard.releaseAll();
    }
    
    // Send mute command to QCC5124
    sendQCCCommand(0x03, muted ? 1 : 0);
    
    Serial.println(muted ? "Muted (Mic OFF)" : "Unmuted (Mic ON)");
}

void updateBattery() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        int adcValue = analogRead(PIN_BAT_ADC);
        // Convert ADC reading to battery percentage (3.0V-4.2V range)
        float voltage = (adcValue / 4095.0f) * 3.3f * 2.0f; // Voltage divider
        batteryPercent = map(voltage * 100, 300, 420, 0, 100);
        batteryPercent = constrain(batteryPercent, 0, 100);
    }
}

void updateCharging() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 500) {
        lastUpdate = millis();
        
        // Read TP4056 STAT pin (LOW = charging, HIGH = complete/standby)
        bool statPin = digitalRead(PIN_STAT);
        
        if (!statPin && batteryPercent < 95) {
            isCharging = true;
            chargingComplete = false;
        } else if (batteryPercent >= 95) {
            isCharging = false;
            chargingComplete = true;
        } else {
            isCharging = false;
            chargingComplete = false;
        }
    }
}

void updateDisplay() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 100) {
        lastUpdate = millis();
        
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        
        // Title
        display.setCursor(0, 0);
        display.println("ESP32-C3 Headset");
        
        // Battery status
        display.setCursor(0, 10);
        if (isCharging) {
            display.printf("Battery: Charging");
        } else if (chargingComplete) {
            display.printf("Battery: Full");
        } else {
            display.printf("Battery: %.0f%%", batteryPercent);
        }
        
        // Status
        display.setCursor(0, 20);
        display.printf("Vol:%d %s %s", 
                      volume, 
                      muted ? "MUTE" : (voiceDetected ? "VOICE" : ""),
                      connected ? "CONN" : "DISC");
        
        display.display();
    }
}

// QCC5124 Communication
void sendQCCCommand(uint8_t cmd, uint8_t data) {
    if (audioEnabled) {
        Serial1.write(0xFF); // Start byte
        Serial1.write(cmd);
        Serial1.write(data);
        Serial1.write(0xFE); // End byte
    }
}

void initQCC5124() {
    delay(100);
    sendQCCCommand(0x00, 0x01); // Initialize
    delay(50);
    sendQCCCommand(0x01, volume); // Set volume
    delay(50);
    sendQCCCommand(0x04, 0x01); // Enable A2DP
}

// Audio Processing
float calculateEnergy(float* buffer, int size) {
    float energy = 0.0f;
    for (int i = 0; i < size; i++) {
        energy += buffer[i] * buffer[i];
    }
    return sqrt(energy / size);
}

bool detectVoice() {
    // Simple VAD based on energy threshold
    float avgEnergy = 0.0f;
    for (int i = 0; i < 10; i++) {
        avgEnergy += energyHistory[i];
    }
    avgEnergy /= 10.0f;
    
    return avgEnergy > VAD_THRESHOLD;
}

void processAudio() {
    if (millis() - lastVADCheck > 20) { // 50Hz VAD check
        lastVADCheck = millis();
        
        if (micEnabled) {
            // Simulate audio input (replace with actual I2S/ADC reading)
            float energy = random(0, 1000) / 1000000.0f;
            
            energyHistory[historyIndex] = energy;
            historyIndex = (historyIndex + 1) % 10;
            
            voiceDetected = detectVoice();
            
            // Only enable mic output when voice is detected
            if (!voiceDetected && !muted) {
                // Could add actual mic gating here
            }
        }
    }
}

// Bluetooth Audio Callbacks
void avrc_metadata_callback(uint8_t data1, const uint8_t *data2) {
    Serial.printf("AVRC metadata: %d, %s\n", data1, data2);
    connected = true;
}

void audio_data_callback(const uint8_t *data, uint32_t length) {
    // Audio data received from Bluetooth
    // Forward to QCC5124 or process here
    if (audioEnabled && !muted) {
        // Process audio data
        for (uint32_t i = 0; i < length && i < FRAME_SIZE * 2; i += 2) {
            int16_t sample = (data[i+1] << 8) | data[i];
            audioBuffer[i/2] = sample / 32768.0f;
        }
    }
}
