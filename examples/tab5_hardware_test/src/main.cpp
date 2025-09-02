/**
 * Tab5duino-idf Hardware Test
 * 
 * Test Tab5duino framework with M5Stack Tab5 hardware
 * Validates framework functionality and hardware integration
 */

#include <Tab5duino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("\n" + String("=").substring(0, 50));
    Serial.println("Tab5duino-idf Hardware Test");
    Serial.println(String("=").substring(0, 50));
    
    // Initialize Tab5duino framework
    Serial.println("Initializing Tab5duino framework...");
    Tab5duino.begin();
    
    // Test framework subsystems
    Serial.printf("Framework Version: %s\n", Tab5duino.getVersion().c_str());
    Serial.printf("Hardware: %s\n", Tab5duino.getHardware().c_str());
    Serial.printf("Free Heap: %lu bytes\n", Tab5duino.getFreeHeap());
    Serial.printf("Free PSRAM: %lu bytes\n", Tab5duino.getFreePsram());
    
    // Test GPIO
    Serial.println("\nTesting GPIO...");
    pinMode(2, OUTPUT);  // Built-in LED or GPIO pin
    
    // Test display subsystem
    if (Tab5duino.hasSubsystem(TAB5_SUBSYSTEM_DISPLAY)) {
        Serial.println("Display subsystem: AVAILABLE");
        Serial.printf("Display Resolution: %dx%d\n", 
                     Tab5duino.Display.getWidth(), 
                     Tab5duino.Display.getHeight());
    } else {
        Serial.println("Display subsystem: NOT AVAILABLE");
    }
    
    // Test touch subsystem  
    if (Tab5duino.hasSubsystem(TAB5_SUBSYSTEM_TOUCH)) {
        Serial.println("Touch subsystem: AVAILABLE");
    } else {
        Serial.println("Touch subsystem: NOT AVAILABLE");
    }
    
    // Test IMU subsystem
    if (Tab5duino.hasSubsystem(TAB5_SUBSYSTEM_IMU)) {
        Serial.println("IMU subsystem: AVAILABLE");
    } else {
        Serial.println("IMU subsystem: NOT AVAILABLE");
    }
    
    // Test LVGL subsystem
    if (Tab5duino.hasSubsystem(TAB5_SUBSYSTEM_LVGL)) {
        Serial.println("LVGL subsystem: AVAILABLE");
        Serial.println("LVGL graphics ready for use!");
    } else {
        Serial.println("LVGL subsystem: NOT AVAILABLE");
    }
    
    Serial.println(String("=").substring(0, 50));
    Serial.println("Tab5duino framework initialized successfully!");
    Serial.println("Starting main loop...\n");
}

void loop() {
    // Update framework (handles LVGL, touch, etc.)
    Tab5duino.update();
    
    // Simple GPIO test - blink LED
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastBlink > 1000) {
        ledState = !ledState;
        digitalWrite(2, ledState);
        lastBlink = millis();
        
        // Print status
        Serial.printf("[%lu] ALIVE | Free Heap: %lu | Free PSRAM: %lu\n",
                     millis(), Tab5duino.getFreeHeap(), Tab5duino.getFreePsram());
    }
    
    // Test touch input
    if (Tab5duino.hasSubsystem(TAB5_SUBSYSTEM_TOUCH)) {
        if (Tab5duino.Touch.wasPressed()) {
            Serial.printf("Touch detected at (%d, %d)\n", 
                         Tab5duino.Touch.getPressX(), 
                         Tab5duino.Touch.getPressY());
        }
    }
    
    // Test IMU
    if (Tab5duino.hasSubsystem(TAB5_SUBSYSTEM_IMU)) {
        if (Tab5duino.IMU.wasKnocked()) {
            Serial.println("Knock detected on device!");
        }
    }
    
    delay(10);  // Small delay for stability
}