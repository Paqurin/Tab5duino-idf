# Tab5duino-IDF Framework

Arduino-compatible framework for ESP32-P4 M5Stack Tab5 built on ESP-IDF foundation.

## Overview

Tab5duino-IDF provides a familiar Arduino development environment while leveraging the full power and reliability of ESP-IDF for the ESP32-P4 RISC-V architecture. Designed specifically for the M5Stack Tab5 with its unique hardware features including 1280x720 display, multi-touch interface, IMU, dual microphones, and solar charging capability.

## Features

### Arduino Compatibility
- **Familiar API**: `setup()`, `loop()`, `pinMode()`, `digitalWrite()`, etc.
- **Arduino Libraries**: Compatible with most Arduino libraries
- **Serial Communication**: Full Serial, Serial1, Serial2 support
- **Standard Functions**: `millis()`, `delay()`, `analogRead()`, etc.

### ESP-IDF Foundation
- **RISC-V Architecture**: Optimized for ESP32-P4 dual-core RISC-V
- **FreeRTOS**: Full multitasking support with thread-safe operations
- **Memory Management**: Advanced 32MB PSRAM utilization
- **Performance**: Hardware acceleration and optimized drivers

### Hardware Abstraction Layer
- **Display HAL**: 1280x720 MIPI-DSI with PPA acceleration
- **Touch HAL**: GT911 multi-touch with gesture recognition
- **IMU HAL**: BMI270 6-axis with motion detection
- **Audio HAL**: PDM microphones with audio processing
- **Power HAL**: Solar charging and advanced power management

### Advanced Features
- **Subsystem Management**: Modular hardware initialization
- **Error Handling**: Professional error reporting and recovery
- **Memory Optimization**: PSRAM-aware memory allocation
- **Power Management**: Multiple power modes with solar support
- **Real-time Performance**: 1280x720@60fps display capability

## Hardware Specifications

| Component | Specification |
|-----------|---------------|
| **MCU** | ESP32-P4 Dual-core RISC-V @ 400MHz |
| **Memory** | 786KB RAM + 32MB PSRAM |
| **Storage** | 16MB Flash |
| **Display** | 1280x720 MIPI-DSI LCD |
| **Touch** | GT911 10-point capacitive |
| **IMU** | BMI270 6-axis (accel + gyro) |
| **Audio** | Dual PDM microphones |
| **Connectivity** | WiFi 6, Bluetooth 5.0, USB-C OTG |
| **Power** | Solar charging with battery backup |

## Installation

### PlatformIO (Recommended)

1. Install the Tab5duino-IDF platform:
```bash
pio platform install file:///path/to/Tab5duino-idf
```

2. Create a new project:
```bash
pio project init --board m5tab5 --project-option "framework=tab5duino-idf"
```

3. Configure `platformio.ini`:
```ini
[env:m5tab5]
platform = tab5duino-idf
board = m5tab5  
framework = tab5duino-idf
monitor_speed = 115200
```

### Manual Installation

1. Clone the repository:
```bash
git clone https://github.com/Paqurin/Tab5duino-idf.git
```

2. Add to PlatformIO platforms directory or use as local platform.

## Quick Start

### Basic Example

```cpp
#include <Tab5duino.h>

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Framework automatically initializes hardware
    Serial.println("Tab5duino-IDF Framework Ready!");
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW); 
    delay(1000);
}
```

### Advanced Framework Usage

```cpp
#include <Tab5duino.h>

void setup() {
    Serial.begin(115200);
    
    // Initialize specific subsystems if needed
    if (Tab5duino.getSubsystemState(TAB5_SUBSYSTEM_IMU) != TAB5_SUBSYSTEM_READY) {
        Tab5duino.initSubsystem(TAB5_SUBSYSTEM_IMU);
    }
    
    Serial.print("Framework Version: ");
    Serial.println(Tab5duino.getVersionString());
    Serial.print("Free PSRAM: ");
    Serial.println(ESP.getFreePsram());
}

void loop() {
    // Your application code here
    // Framework handles hardware management automatically
}
```

## Pin Definitions

```cpp
// Built-in components
#define LED_BUILTIN     2
#define BUTTON_A        0   // Boot button
#define BUTTON_B        46
#define BUTTON_C        45

// Display
#define TFT_BACKLIGHT   13
#define TFT_CS          10
#define TFT_DC          11
#define TFT_RST         12

// Touch (GT911)
#define TOUCH_SDA       6
#define TOUCH_SCL       7
#define TOUCH_INT       8
#define TOUCH_RST       9

// IMU (BMI270)
#define IMU_SDA         4
#define IMU_SCL         5
#define IMU_INT1        14
#define IMU_INT2        15

// Audio
#define MIC_DATA        16  // PDM microphone data
#define MIC_CLK         17  // PDM microphone clock
#define SPK_DATA        18  // Speaker I2S data
#define SPK_BCLK        19  // Speaker bit clock
#define SPK_WS          20  // Speaker word select

// USB OTG
#define USB_DM          26
#define USB_DP          27

// Power management
#define BAT_ADC         1   // Battery voltage ADC
#define SOLAR_ADC       2   // Solar panel voltage ADC
```

## API Reference

### Framework Management

```cpp
class Tab5duinoFramework {
public:
    esp_err_t begin(const tab5duino_config_t* config = nullptr);
    esp_err_t start();
    void end();
    
    // Subsystem management
    esp_err_t initSubsystem(tab5_subsystem_t subsystem);
    tab5_subsystem_state_t getSubsystemState(tab5_subsystem_t subsystem);
    
    // Information
    const char* getVersionString();
    uint64_t getBootTimeUs();
    bool isReady();
    
    // Memory management
    void* malloc(size_t size);  // PSRAM-aware allocation
    void free(void* ptr);
};

extern Tab5duinoFramework& Tab5duino;
```

### Hardware Abstraction Layer

```cpp
// Display HAL
esp_err_t display_hal_init(const display_config_t* config, display_handle_t* handle);
esp_err_t display_hal_clear(display_handle_t handle, uint16_t color);
esp_err_t display_hal_draw_pixel(display_handle_t handle, uint16_t x, uint16_t y, uint16_t color);

// Touch HAL  
esp_err_t touch_hal_init(const touch_config_t* config, touch_handle_t* handle);
esp_err_t touch_hal_read_points(touch_handle_t handle, touch_point_t* points, uint8_t* count);

// IMU HAL
esp_err_t imu_hal_init(const imu_config_t* config, imu_handle_t* handle);
esp_err_t imu_hal_read_accel(imu_handle_t handle, imu_data_t* accel_data);
esp_err_t imu_hal_read_gyro(imu_handle_t handle, imu_data_t* gyro_data);

// Audio HAL
esp_err_t audio_hal_init(const audio_config_t* config, audio_handle_t* handle);
esp_err_t audio_hal_mic_read(audio_handle_t handle, audio_buffer_t* buffer, uint32_t timeout_ms);

// Power HAL
esp_err_t power_hal_init(const power_config_t* config, power_handle_t* handle);
esp_err_t power_hal_get_battery_info(power_handle_t handle, power_battery_info_t* info);
```

## Configuration

### Framework Configuration

```cpp
tab5duino_config_t config = {
    .auto_init_display = true,
    .auto_init_touch = true, 
    .auto_init_imu = true,
    .auto_init_audio = false,
    .enable_psram = true,
    .enable_usb_serial = true,
    .loop_stack_size = 8192,
    .loop_task_priority = 1,
    .loop_task_core = 1
};

Tab5duino.begin(&config);
```

### Build Configuration

```ini
[env:m5tab5]
platform = tab5duino-idf
board = m5tab5
framework = tab5duino-idf

build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DPSRAM_SIZE_MB=32
    -DFLASH_SIZE_MB=16
    -DARDUINO_USB_CDC_ON_BOOT=1

monitor_speed = 115200
upload_speed = 921600
```

## Examples

- **HelloTab5**: Basic framework usage and hardware testing
- **DisplayTest**: Display and touch functionality
- **IMUDemo**: Motion detection and gesture recognition
- **AudioRecord**: Microphone recording and processing
- **SolarMonitor**: Solar panel and battery monitoring
- **AdvancedGUI**: LVGL integration example

## Performance Considerations

### Memory Management
- **PSRAM Utilization**: Use `Tab5duino.malloc()` for large allocations
- **Stack Size**: Adjust `loop_stack_size` based on application needs
- **Heap Monitoring**: Monitor heap usage with `ESP.getFreeHeap()`

### Real-time Performance
- **Display Refresh**: 60fps capability at 1280x720
- **Touch Response**: <10ms touch response time
- **Audio Processing**: Real-time audio processing support
- **IMU Sampling**: Up to 1600Hz sampling rate

### Power Optimization
- **Power Modes**: Multiple power modes for different scenarios
- **Solar Charging**: Automatic solar power management
- **Sleep Modes**: Deep sleep and hibernation support

## Troubleshooting

### Common Issues

1. **Build Errors**
   - Ensure ESP-IDF 5.5+ is installed
   - Check RISC-V toolchain installation
   - Verify PlatformIO platform installation

2. **Upload Issues**
   - Use correct USB-C cable (data capable)
   - Press and hold Boot button during upload if needed
   - Check upload speed settings

3. **Runtime Issues**
   - Monitor serial output for error messages
   - Check subsystem initialization status
   - Verify hardware connections

### Debug Configuration

```ini
build_flags = 
    -DCORE_DEBUG_LEVEL=5
    -DLOG_DEFAULT_LEVEL=5
    -DDEBUG_ESP_PORT=Serial
    -DDEBUG_ESP_CORE
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes with comprehensive tests
4. Submit pull request with detailed description

## License

MIT License - see LICENSE file for details.

## Support

- **Documentation**: [docs.m5stack.com](https://docs.m5stack.com)
- **Community**: [community.m5stack.com](https://community.m5stack.com)
- **Issues**: [GitHub Issues](https://github.com/Paqurin/Tab5duino-idf/issues)
- **Discord**: [M5Stack Discord](https://discord.gg/m5stack)

## Roadmap

- [ ] Complete HAL driver implementations
- [ ] LVGL integration optimization
- [ ] Advanced power management features
- [ ] OTA update support
- [ ] Bluetooth mesh networking
- [ ] Machine learning inference support
- [ ] Extended library ecosystem