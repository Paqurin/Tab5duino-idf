# ESP32-P4 PlatformIO Upload Fix

## Problem
PlatformIO's standard ESP32 platform doesn't properly support ESP32-P4 hardware uploads, resulting in:
```
A fatal error occurred: This chip is ESP32-P4, not ESP32. Wrong --chip argument?
```

## Solution
Tab5duino-IDF framework includes ESP32-P4 specific fixes:

### 1. ESP32-P4 Board Definition
Use the M5Stack Tab5 P4 board with correct chip configuration:

```ini
[env:m5tab5]
platform = espressif32@^6.5.0  
board = m5tab5_p4               ; Use ESP32-P4 specific board
framework = arduino
lib_deps = 
    https://github.com/Paqurin/Tab5duino-idf.git
```

### 2. Custom Upload Script
The framework includes `builder/upload_esp32p4.py` that automatically:
- Detects ESP32-P4 hardware
- Uses correct `--chip esp32p4` parameter
- Sets appropriate flash settings for M5Stack Tab5

### 3. Manual Upload (Alternative)
If needed, you can upload manually:

```bash
# Build first
pio run

# Upload with correct ESP32-P4 parameters  
esptool.py --chip esp32p4 --port /dev/ttyACM1 --baud 460800 write_flash --flash_mode qio --flash_freq 80m --flash_size 16MB 0x0000 .pio/build/m5tab5/firmware.bin
```

## Benefits
- **Automatic Detection**: Framework handles ESP32-P4 uploads seamlessly
- **Proper Flash Settings**: Optimized for M5Stack Tab5 16MB flash
- **No Manual Intervention**: Works with standard `pio run --target upload`
- **Universal Compatibility**: Fixes apply to all Tab5duino-IDF projects

## Integration
This fix is automatically enabled when using Tab5duino-IDF framework. No additional configuration needed.

## Hardware Verified
✅ Tested on M5Stack Tab5 ESP32-P4 RISC-V hardware  
✅ Upload speed: 460800 baud  
✅ Flash size: 16MB with QIO mode  
✅ Compatible with Tab5duino Arduino API