/**
 * Example Usage of ESP32-C3 Bluetooth Headset
 * 
 * This example demonstrates how to use the key features of the headset firmware:
 * - Battery monitoring and display
 * - Button handling with different press types
 * - Audio control and QCC5124 management
 * - Voice activity detection and noise suppression
 * - USB HID control for Teams/Discord mute
 * 
 * To use this example:
 * 1. Upload the main firmware to your ESP32-C3
 * 2. Connect the hardware as described in SETUP.md
 * 3. Power on the system and observe the OLED display
 * 4. Test buttons and audio functionality
 */

#include <Arduino.h>
// All the main functionality is in src/main.cpp
// This is just a demonstration of the API usage

// Example of custom button event handling
void handleCustomButtonEvent(int button, bool longPress) {
    switch(button) {
        case 0: // Power button
            if (longPress) {
                Serial.println("Power button long press - shutting down");
                // Custom shutdown sequence
            } else {
                Serial.println("Power button short press - toggle audio");
                // Custom audio toggle
            }
            break;
            
        case 1: // Volume up
            Serial.println("Volume up pressed");
            // Custom volume control
            break;
            
        case 2: // Volume down
            Serial.println("Volume down pressed");
            // Custom volume control
            break;
            
        case 3: // Mute button
            if (longPress) {
                Serial.println("Mute long press - Teams/Discord mute");
                // Send USB HID command
            } else {
                Serial.println("Mute short press - local mic mute");
                // Local mic control
            }
            break;
    }
}

// Example of custom audio processing callback
void processAudioFrame(int16_t* inputBuffer, int16_t* outputBuffer, size_t samples) {
    // This would be called from the audio processing task
    // Apply custom audio effects here
    
    // Example: Simple gain control
    float gain = 1.0f;
    for (size_t i = 0; i < samples; i++) {
        outputBuffer[i] = (int16_t)(inputBuffer[i] * gain);
    }
}

// Example of custom VAD callback
void vadCallback(bool voiceActive, float vadScore) {
    // This would be called when voice activity state changes
    Serial.print("Voice Activity: ");
    Serial.print(voiceActive ? "ACTIVE" : "INACTIVE");
    Serial.print(" (Score: ");
    Serial.print(vadScore);
    Serial.println(")");
}

// Example of custom display update
void updateCustomDisplay() {
    // This would be called from the display task
    // Add custom information to the OLED display
    
    // Example: Add system uptime
    unsigned long uptime = millis() / 1000;
    Serial.print("System uptime: ");
    Serial.print(uptime);
    Serial.println(" seconds");
}

// Example of battery monitoring callback
void batteryCallback(float voltage, uint8_t percentage, bool charging) {
    // This would be called when battery status changes
    Serial.print("Battery: ");
    Serial.print(voltage);
    Serial.print("V (");
    Serial.print(percentage);
    Serial.print("%) ");
    Serial.println(charging ? "CHARGING" : "NOT CHARGING");
    
    // Example: Custom low battery handling
    if (percentage < 15 && !charging) {
        Serial.println("WARNING: Low battery!");
        // Could trigger LED blink, display warning, etc.
    }
}

// Example of QCC5124 status callback
void qccStatusCallback(const String& status) {
    Serial.print("QCC5124 Status: ");
    Serial.println(status);
    
    // Example: Custom handling of codec events
    if (status.indexOf("CONNECTED") >= 0) {
        Serial.println("Bluetooth device connected");
        // Could update display, play sound, etc.
    } else if (status.indexOf("DISCONNECTED") >= 0) {
        Serial.println("Bluetooth device disconnected");
        // Could enter pairing mode, update display, etc.
    }
}

// This is not a runnable example - the actual functionality
// is implemented in src/main.cpp. This just shows how you
// could extend or customize the system.

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-C3 Bluetooth Headset Example");
    Serial.println("The actual firmware is in src/main.cpp");
    Serial.println("This file shows example usage patterns");
}

void loop() {
    // Main loop is implemented in src/main.cpp
    delay(1000);
}
