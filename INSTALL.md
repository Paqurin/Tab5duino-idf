# Tab5duino-IDF Installation Guide

Complete installation guide for the Tab5duino-IDF framework - Arduino-compatible framework for ESP32-P4 M5Stack Tab5.

## Prerequisites

### Hardware Requirements
- **M5Stack Tab5** device with ESP32-P4
- **USB-C cable** (data-capable, not just charging)
- **Computer** with Windows 10+, macOS 10.15+, or Linux

### Software Requirements
- **Python 3.7+** (required for PlatformIO)
- **Git** (for repository management)
- **Code Editor** (VS Code with PlatformIO extension recommended)

## Installation Methods

### Method 1: PlatformIO Platform Installation (Recommended)

This method installs Tab5duino-IDF as a PlatformIO platform for easy project management.

#### 1. Install PlatformIO Core

**Option A: Via Python pip**
```bash
pip install platformio
```

**Option B: Via Homebrew (macOS)**
```bash
brew install platformio
```

**Option C: Via VS Code Extension**
- Install "PlatformIO IDE" extension in VS Code
- PlatformIO Core will be installed automatically

#### 2. Install Tab5duino-IDF Platform

**From GitHub Release:**
```bash
# Download and install from GitHub
pio platform install https://github.com/Paqurin/Tab5duino-idf/archive/v1.0.0.tar.gz
```

**From Local Copy:**
```bash
# Clone repository first
git clone https://github.com/Paqurin/Tab5duino-idf.git
cd Tab5duino-idf

# Install as local platform
pio platform install file://$(pwd)
```

#### 3. Create New Project

```bash
# Create project directory
mkdir my-tab5-project
cd my-tab5-project

# Initialize PlatformIO project
pio project init --board m5tab5 --project-option "framework=tab5duino-idf"
```

#### 4. Configure Project

Edit `platformio.ini`:
```ini
[env:m5tab5]
platform = tab5duino-idf
board = m5tab5
framework = tab5duino-idf
monitor_speed = 115200

; Optional build flags
build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DPSRAM_SIZE_MB=32
    -DARDUINO_USB_CDC_ON_BOOT=1

; Upload settings
upload_speed = 921600
upload_port = auto
```

### Method 2: Direct Repository Usage

Use the repository directly without installing as a platform.

#### 1. Clone Repository

```bash
git clone https://github.com/Paqurin/Tab5duino-idf.git
cd Tab5duino-idf
```

#### 2. Use Example Projects

```bash
# Navigate to example
cd examples/HelloTab5

# Build and upload
pio run --target upload

# Monitor serial output
pio device monitor
```

#### 3. Create Custom Project

```bash
# Copy example as starting point
cp -r examples/HelloTab5 ../my-project
cd ../my-project

# Modify platformio.ini to point to framework
```

```ini
[env:m5tab5]
platform = file:///path/to/Tab5duino-idf
board = m5tab5
framework = tab5duino-idf
monitor_speed = 115200
```

### Method 3: Development Setup

For contributing to or modifying the framework itself.

#### 1. Fork and Clone

```bash
# Fork repository on GitHub first, then:
git clone https://github.com/yourusername/Tab5duino-idf.git
cd Tab5duino-idf
```

#### 2. Install Development Dependencies

```bash
# Install PlatformIO
pip install platformio

# Install additional development tools
pip install pre-commit black isort
```

#### 3. Test Framework

```bash
# Build all examples
cd examples/HelloTab5
pio run

cd ../tab5_hardware_test  
pio run

cd ../lvgl_basic
pio run
```

## Quick Start

### 1. Basic Arduino-Style Program

Create `src/main.cpp`:

```cpp
#include <Tab5duino.h>

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.println("Tab5duino-IDF Framework Ready!");
    Serial.print("Framework Version: ");
    Serial.println(Tab5duino.getVersionString());
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    
    Serial.println("Hello from Tab5!");
}
```

### 2. Build and Upload

```bash
# Build project
pio run

# Upload to device (connect M5Stack Tab5)
pio run --target upload

# Monitor serial output
pio device monitor
```

### 3. Advanced Framework Usage

```cpp
#include <Tab5duino.h>

void setup() {
    Serial.begin(115200);
    
    // Custom framework configuration
    tab5duino_config_t config = {
        .auto_init_display = true,
        .auto_init_touch = true,
        .auto_init_imu = true,
        .enable_psram = true,
        .loop_stack_size = 8192
    };
    
    Tab5duino.begin(&config);
    
    // Check subsystem status
    if (Tab5duino.getSubsystemState(TAB5_SUBSYSTEM_DISPLAY) == TAB5_SUBSYSTEM_READY) {
        Serial.println("Display ready!");
    }
}

void loop() {
    // Your application code here
    Tab5duino.update(); // Update framework services
}
```

## Hardware Setup

### 1. Driver Installation

**Windows:**
- Install CH340/CH341 USB drivers if needed
- Device should appear as COM port

**macOS:**
- Usually works out-of-the-box
- Install drivers if device not recognized

**Linux:**
- Add user to dialout group: `sudo usermod -a -G dialout $USER`
- Logout and login again

### 2. Connection

1. Connect M5Stack Tab5 to computer via USB-C cable
2. Verify connection: `pio device list`
3. Note the device port (e.g., `/dev/ttyUSB0`, `COM3`)

### 3. Upload Test

```bash
# Test upload with simple program
cd examples/HelloTab5
pio run --target upload --upload-port /dev/ttyUSB0
```

## IDE Configuration

### VS Code with PlatformIO

1. Install VS Code
2. Install "PlatformIO IDE" extension
3. Open Tab5duino-IDF folder or create new project
4. Use PlatformIO toolbar for build/upload operations

### Arduino IDE (Compatibility Mode)

While Tab5duino-IDF is designed for PlatformIO, basic Arduino IDE compatibility:

1. Copy framework files to Arduino libraries folder
2. Use `.ino` files with `#include <Tab5duino.h>`
3. Limited functionality compared to PlatformIO

## Troubleshooting

### Build Issues

**Error: Platform not found**
```bash
# Verify platform installation
pio platform list | grep tab5duino

# Reinstall platform
pio platform uninstall tab5duino-idf
pio platform install https://github.com/Paqurin/Tab5duino-idf/archive/v1.0.0.tar.gz
```

**Error: Board not found**
```bash
# Check board definition
pio boards | grep m5tab5

# Verify platform includes board definition
```

**Error: Framework not found**
```bash
# Check platformio.ini configuration
# Ensure framework = tab5duino-idf
```

### Upload Issues

**Device not found**
```bash
# List available devices
pio device list

# Specify port manually
pio run --target upload --upload-port /dev/ttyUSB0
```

**Permission denied (Linux)**
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Or run with sudo (not recommended)
sudo pio run --target upload
```

**Upload timeout**
```bash
# Hold BOOT button during upload
# Try different upload speed
upload_speed = 115200  # in platformio.ini
```

### Runtime Issues

**Framework not initializing**
- Check serial output for error messages
- Verify hardware connections
- Check power supply adequacy
- Ensure PSRAM is properly configured

**Subsystem errors**
- Monitor serial output for HAL initialization errors
- Check hardware component connections
- Verify GPIO pin definitions

### Performance Issues

**Slow build times**
```bash
# Use ccache (if available)
export PLATFORMIO_BUILD_CACHE_DIR=~/.platformio_cache

# Parallel builds
build_flags = -j8
```

**Memory issues**
- Monitor heap usage with `ESP.getFreeHeap()`
- Use PSRAM for large allocations: `Tab5duino.malloc()`
- Optimize build flags for memory efficiency

## Migration Guide

### From Arduino Framework

1. **Replace includes:**
   ```cpp
   // Old
   #include "Arduino.h"
   
   // New  
   #include <Tab5duino.h>
   ```

2. **Update platformio.ini:**
   ```ini
   ; Old
   framework = arduino
   
   ; New
   framework = tab5duino-idf
   ```

3. **Leverage framework features:**
   ```cpp
   // Access Tab5duino-specific features
   Tab5duino.begin();
   Tab5duino.getVersionString();
   ```

### From ESP-IDF

1. **Keep ESP-IDF components:**
   - Full ESP-IDF functionality available
   - Use familiar ESP-IDF APIs alongside Arduino APIs

2. **Simplified initialization:**
   ```cpp
   // ESP-IDF style still works
   #include "esp_system.h"
   #include "driver/gpio.h"
   
   // Plus Arduino convenience
   #include <Tab5duino.h>
   pinMode(2, OUTPUT);
   ```

## Advanced Configuration

### Custom Board Definition

Create custom board in `boards/` directory:

```json
{
  "build": {
    "arduino": {
      "ldscript": "esp32p4_out.ld"
    },
    "core": "tab5duino",
    "extra_flags": "-DARDUINO_M5TAB5_CUSTOM",
    "f_cpu": "400000000L",
    "f_flash": "80000000L",
    "flash_mode": "dio",
    "hwids": [["0x303A", "0x1001"]],
    "mcu": "esp32p4",
    "variant": "m5tab5_custom"
  },
  "connectivity": ["wifi", "bluetooth"],
  "debug": {
    "openocd_target": "esp32p4.cfg"
  },
  "frameworks": ["tab5duino-idf"],
  "name": "M5Stack Tab5 Custom",
  "upload": {
    "flash_size": "16MB",
    "maximum_ram_size": 786432,
    "maximum_size": 16777216,
    "require_upload_port": true,
    "speed": 921600
  },
  "url": "https://docs.m5stack.com/en/core/M5Tab5",
  "vendor": "M5Stack"
}
```

### Framework Configuration

Advanced framework configuration:

```cpp
tab5duino_config_t config = {
    .version = TAB5DUINO_CONFIG_VERSION,
    .auto_init_display = true,
    .auto_init_touch = true,
    .auto_init_imu = true,
    .auto_init_audio = false,
    .enable_psram = true,
    .enable_usb_serial = true,
    .loop_stack_size = 16384,
    .loop_task_priority = 1,
    .loop_task_core = 1,
    .log_level = ESP_LOG_INFO,
    .watchdog_timeout_ms = 30000
};

Tab5duino.begin(&config);
```

### Build Optimization

Performance and size optimizations:

```ini
[env:m5tab5_optimized]
platform = tab5duino-idf
board = m5tab5
framework = tab5duino-idf

build_flags = 
    -O3                          ; Maximum optimization
    -DCORE_DEBUG_LEVEL=0         ; Disable debug output
    -DPSRAM_SIZE_MB=32          ; Full PSRAM utilization
    -ffunction-sections         ; Function-level linking
    -fdata-sections             ; Data-level linking
    -Wl,--gc-sections           ; Dead code elimination
    -DARDUINO_LOOP_STACK_SIZE=16384

build_unflags = -Os              ; Remove size optimization
```

## Support and Resources

### Documentation
- **Framework API**: See `framework/cores/tab5duino/` headers
- **Examples**: Explore `examples/` directory
- **Hardware Reference**: M5Stack Tab5 documentation

### Community
- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Questions and community support
- **Discord**: Real-time discussion and support

### Development
- **Contributing**: See `CONTRIBUTING.md`
- **Source Code**: Full source available on GitHub
- **License**: MIT License (see `LICENSE`)

---

## Need Help?

If you encounter issues:

1. **Check this guide** for common solutions
2. **Search GitHub Issues** for similar problems  
3. **Create new issue** with detailed information:
   - Hardware version
   - Software versions
   - Complete error messages
   - Minimal reproduction case

Welcome to Tab5duino-IDF development!