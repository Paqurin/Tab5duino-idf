/*
 * HelloTab5 Example for Tab5duino-IDF Framework
 * Demonstrates basic framework initialization and hardware access
 * 
 * This example shows:
 * - Framework initialization
 * - LED control
 * - Button reading
 * - Serial communication
 * - Battery monitoring
 * - IMU data reading
 * - Touch detection
 */

#include <Tab5duino.h>

// Global variables
bool ledState = false;
unsigned long lastBlink = 0;
unsigned long lastSensorRead = 0;
unsigned long lastBatteryCheck = 0;

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for Serial to be ready
  }
  
  Serial.println();
  Serial.println("==============================");
  Serial.println("  Tab5duino-IDF Framework");
  Serial.println("  Hello Tab5 Example");
  Serial.println("==============================");
  Serial.print("Framework Version: ");
  Serial.println(Tab5duino.getVersionString());
  Serial.print("Boot Time: ");
  Serial.print(Tab5duino.getBootTimeUs());
  Serial.println(" μs");
  Serial.println();

  // Initialize GPIO pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  
  // Initialize display backlight
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Turn on backlight
  
  // Initialize framework subsystems manually if needed
  // (most are auto-initialized based on configuration)
  
  // Check if all subsystems are ready
  Serial.println("Subsystem Status:");
  Serial.print("  Display: ");
  Serial.println(Tab5duino.getSubsystemState(TAB5_SUBSYSTEM_DISPLAY) == TAB5_SUBSYSTEM_READY ? "Ready" : "Not Ready");
  Serial.print("  Touch: ");
  Serial.println(Tab5duino.getSubsystemState(TAB5_SUBSYSTEM_TOUCH) == TAB5_SUBSYSTEM_READY ? "Ready" : "Not Ready");
  Serial.print("  IMU: ");
  Serial.println(Tab5duino.getSubsystemState(TAB5_SUBSYSTEM_IMU) == TAB5_SUBSYSTEM_READY ? "Ready" : "Not Ready");
  Serial.print("  Power: ");
  Serial.println(Tab5duino.getSubsystemState(TAB5_SUBSYSTEM_POWER) == TAB5_SUBSYSTEM_READY ? "Ready" : "Not Ready");
  Serial.println();
  
  Serial.println("Setup complete! Starting main loop...");
  Serial.println("Press buttons A, B, or C to test input");
  Serial.println("LED will blink every second");
  Serial.println("Sensor data will be displayed every 5 seconds");
  Serial.println();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Blink LED every second
  if (currentTime - lastBlink >= 1000) {
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
    lastBlink = currentTime;
    
    Serial.print("LED: ");
    Serial.println(ledState ? "ON" : "OFF");
  }
  
  // Check buttons
  checkButtons();
  
  // Read sensors every 5 seconds
  if (currentTime - lastSensorRead >= 5000) {
    readSensors();
    lastSensorRead = currentTime;
  }
  
  // Check battery every 30 seconds
  if (currentTime - lastBatteryCheck >= 30000) {
    checkBattery();
    lastBatteryCheck = currentTime;
  }
  
  // Small delay to prevent overwhelming the system
  delay(50);
}

void checkButtons() {
  static bool buttonA_last = HIGH;
  static bool buttonB_last = HIGH;
  static bool buttonC_last = HIGH;
  
  bool buttonA = digitalRead(BUTTON_A);
  bool buttonB = digitalRead(BUTTON_B);
  bool buttonC = digitalRead(BUTTON_C);
  
  // Check for button presses (falling edge)
  if (buttonA != buttonA_last && buttonA == LOW) {
    Serial.println("Button A pressed!");
    // Toggle backlight
    static bool backlightOn = true;
    backlightOn = !backlightOn;
    digitalWrite(TFT_BACKLIGHT, backlightOn);
    Serial.print("Backlight: ");
    Serial.println(backlightOn ? "ON" : "OFF");
  }
  
  if (buttonB != buttonB_last && buttonB == LOW) {
    Serial.println("Button B pressed!");
    // Print system information
    printSystemInfo();
  }
  
  if (buttonC != buttonC_last && buttonC == LOW) {
    Serial.println("Button C pressed!");
    // Reset framework statistics or perform other action
    Serial.println("Framework action triggered!");
  }
  
  buttonA_last = buttonA;
  buttonB_last = buttonB;
  buttonC_last = buttonC;
}

void readSensors() {
  Serial.println("--- Sensor Readings ---");
  
  // Read analog values (battery and solar if available)
  int batteryRaw = analogRead(A0);
  int solarRaw = analogRead(A1);
  
  Serial.print("Battery ADC: ");
  Serial.print(batteryRaw);
  Serial.print(" (");
  Serial.print(batteryRaw * 3.3 / 4095.0, 2);
  Serial.println("V)");
  
  Serial.print("Solar ADC: ");
  Serial.print(solarRaw);
  Serial.print(" (");
  Serial.print(solarRaw * 3.3 / 4095.0, 2);
  Serial.println("V)");
  
  // Note: Actual IMU and touch reading would require HAL implementations
  // For now, we'll show placeholder values
  Serial.println("IMU: [Placeholder - requires HAL implementation]");
  Serial.println("Touch: [Placeholder - requires HAL implementation]");
  
  Serial.println("----------------------");
}

void checkBattery() {
  Serial.println("--- Battery Status ---");
  
  // Read battery voltage (placeholder implementation)
  int batteryRaw = analogRead(A0);
  float batteryVoltage = batteryRaw * 3.3 / 4095.0 * 2.0; // Assuming voltage divider
  
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage, 2);
  Serial.println("V");
  
  // Estimate battery percentage (very rough estimate)
  float batteryPercent = constrain(map(batteryVoltage * 100, 320, 420, 0, 100), 0, 100);
  Serial.print("Battery Level: ");
  Serial.print(batteryPercent, 0);
  Serial.println("%");
  
  // Check solar panel
  int solarRaw = analogRead(A1);
  float solarVoltage = solarRaw * 3.3 / 4095.0;
  Serial.print("Solar Panel: ");
  Serial.print(solarVoltage, 2);
  Serial.print("V");
  if (solarVoltage > 1.0) {
    Serial.println(" (Active)");
  } else {
    Serial.println(" (Inactive)");
  }
  
  Serial.println("---------------------");
}

void printSystemInfo() {
  Serial.println();
  Serial.println("=== System Information ===");
  Serial.print("Framework: Tab5duino-IDF v");
  Serial.println(Tab5duino.getVersionString());
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  if (psramFound()) {
    Serial.print("PSRAM Size: ");
    Serial.print(ESP.getPsramSize());
    Serial.println(" bytes");
    Serial.print("Free PSRAM: ");
    Serial.print(ESP.getFreePsram());
    Serial.println(" bytes");
  }
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize());
  Serial.println(" bytes");
  Serial.println("===========================");
  Serial.println();
}

// Framework hooks (optional)
void on_framework_init() {
  Serial.println("Framework initialization hook called");
}

void on_framework_ready() {
  Serial.println("Framework ready hook called");
}

void on_subsystem_error(tab5_subsystem_t subsystem, esp_err_t error) {
  Serial.print("Subsystem error - Subsystem: ");
  Serial.print(subsystem);
  Serial.print(", Error: ");
  Serial.println(esp_err_to_name(error));
}