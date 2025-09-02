# LVGL Migration Guide for Tab5duino Framework

This guide helps you migrate existing LVGL projects to the Tab5duino framework and take advantage of the ESP32-P4 hardware acceleration features.

## Quick Start

### Basic Integration

Replace your existing LVGL initialization with Tab5duino integration:

```cpp
// Old LVGL code
#include "lvgl.h"
lv_init();
// Manual display/touch driver setup...

// New Tab5duino + LVGL code  
#include <Tab5duino.h>
#include <lvgl_tab5duino.h>

extern LVGLTab5duino& LVGL;

void setup() {
    Tab5duino.begin();        // Initialize framework
    LVGL.begin();            // Initialize LVGL integration
    LVGL.start();            // Start LVGL task
}
```

### Key Differences

| Aspect | Standard LVGL | Tab5duino LVGL |
|--------|---------------|----------------|
| **Display Setup** | Manual driver init | Automatic HAL integration |
| **Touch Input** | Manual driver init | Automatic GT911 integration |
| **Memory Management** | Manual PSRAM config | Optimized 32MB PSRAM usage |
| **Task Management** | Manual task creation | Automatic task with mutex |
| **Hardware Acceleration** | Manual PPA setup | Automatic PPA integration |
| **Thread Safety** | Manual locking | Built-in lock/unlock methods |

## Migration Steps

### Step 1: Update Configuration

**Old lv_conf.h:**
```c
#define LV_COLOR_DEPTH 16
#define LV_MEM_SIZE (64 * 1024)
#define LV_DISP_DEF_REFR_PERIOD 30
```

**New lv_conf.h (provided by Tab5duino):**
```c
#define LV_COLOR_DEPTH 16
#define LV_MEM_CUSTOM 1
#define LV_DISP_DEF_REFR_PERIOD 16    // 60fps
#define LV_USE_LARGE_COORD 1          // For 1280x720
```

### Step 2: Update Display Driver

**Old Code:**
```cpp
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DISPLAY_WIDTH * 10];

void display_init() {
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, DISPLAY_WIDTH * 10);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISPLAY_WIDTH;
    disp_drv.ver_res = DISPLAY_HEIGHT;
    disp_drv.flush_cb = my_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}
```

**New Code:**
```cpp
// No manual setup needed! Tab5duino handles it automatically
void on_lvgl_ready() {
    // LVGL is ready, create your UI here
    create_my_ui();
}
```

### Step 3: Update Touch Input

**Old Code:**
```cpp
static lv_indev_drv_t indev_drv;
lv_indev_drv_init(&indev_drv);
indev_drv.type = LV_INDEV_TYPE_POINTER;
indev_drv.read_cb = touchpad_read;
lv_indev_drv_register(&indev_drv);
```

**New Code:**
```cpp
// Touch input handled automatically by Tab5duino
// Multi-touch and gestures supported out of the box
```

### Step 4: Update Thread Safety

**Old Code:**
```cpp
// Manual mutex handling
SemaphoreHandle_t lvgl_mutex;

void some_function() {
    xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
    lv_obj_set_pos(obj, x, y);
    xSemaphoreGive(lvgl_mutex);
}
```

**New Code:**
```cpp
// Built-in thread safety
void some_function() {
    LVGL.callSafeVoid([&]() {
        lv_obj_set_pos(obj, x, y);
    });
    
    // Or traditional style:
    if (LVGL.lock()) {
        lv_obj_set_pos(obj, x, y);
        LVGL.unlock();
    }
}
```

### Step 5: Update Memory Management

**Old Code:**
```cpp
#define LV_MEM_CUSTOM 0
// Uses limited internal memory
```

**New Code:**
```cpp
// Automatic PSRAM usage
// 32MB available for frame buffers and UI objects
// Optimized allocation for large displays
```

## Hardware-Specific Optimizations

### ESP32-P4 PPA Acceleration

The Tab5duino LVGL integration automatically enables PPA (Pixel Processing Accelerator):

```cpp
// PPA acceleration is automatic for:
// - Rectangle fills
// - Image blending  
// - Color space conversion
// - Alpha blending operations

// To check if PPA is active:
lvgl_tab5_config_t config = LVGL_TAB5_CONFIG_DEFAULT;
config.enable_ppa_acceleration = true;  // Default
LVGL.begin(&config);
```

### High-Resolution Display (1280x720)

Optimizations for the large display:

```cpp
// Automatic optimizations:
// - Double buffering with PSRAM
// - Efficient line-based rendering
// - VSYNC support for tear-free display
// - Large coordinate system support

// Manual configuration if needed:
lvgl_tab5_config_t config = LVGL_TAB5_CONFIG_DEFAULT;
config.buffer_lines = 120;        // Lines per buffer
config.enable_double_buffer = true;
config.enable_vsync = true;
LVGL.begin(&config);
```

## Performance Optimization

### Memory Usage

```cpp
// Old: Limited internal RAM
lv_obj_t* obj = lv_obj_create(parent);

// New: PSRAM-backed allocations
// Objects automatically use PSRAM when available
// No code changes needed
```

### Frame Rate Optimization

```cpp
// Achieve 60fps with proper configuration:
lvgl_tab5_config_t config = LVGL_TAB5_CONFIG_DEFAULT;
config.tick_period_ms = 5;        // 5ms ticks
config.task_priority = 2;         // High priority LVGL task
config.task_core = 1;            // Dedicated core
LVGL.begin(&config);
```

## Common Patterns

### Screen Management

**Old Pattern:**
```cpp
lv_obj_t* screen = lv_obj_create(NULL);
lv_scr_load(screen);
```

**New Pattern:**
```cpp
lv_obj_t* screen = LVGL.createScreen();  // Thread-safe creation
LVGL.loadScreen(screen);                 // Thread-safe loading
```

### Event Handling

**Old Pattern:**
```cpp
static void btn_event_cb(lv_event_t* e) {
    // Direct LVGL calls (not thread-safe from interrupts)
    lv_label_set_text(label, "Clicked");
}
```

**New Pattern:**
```cpp
static void btn_event_cb(lv_event_t* e) {
    // Already in LVGL task context - safe to call directly
    lv_label_set_text(label, "Clicked");
}

// For calls from other tasks/interrupts:
void external_function() {
    LVGL.callSafeVoid([&]() {
        lv_label_set_text(label, "Updated");
    });
}
```

### Performance Monitoring

```cpp
void loop() {
    if (LVGL.isReady()) {
        uint16_t fps;
        uint8_t cpu_usage;
        uint32_t mem_used;
        
        LVGL.getPerformanceStats(&fps, &cpu_usage, &mem_used);
        Serial.printf("FPS: %d, CPU: %d%%, Memory: %lu KB\n", 
                     fps, cpu_usage, mem_used / 1024);
    }
}
```

## Troubleshooting

### Common Issues

1. **Screen flickering:**
   ```cpp
   // Enable VSYNC
   lvgl_tab5_config_t config = LVGL_TAB5_CONFIG_DEFAULT;
   config.enable_vsync = true;
   LVGL.begin(&config);
   ```

2. **Low frame rate:**
   ```cpp
   // Optimize buffer size and enable PPA
   config.buffer_lines = 120;
   config.enable_ppa_acceleration = true;
   config.tick_period_ms = 5;
   ```

3. **Touch not responding:**
   ```cpp
   // Check if touch is enabled in Tab5duino config
   tab5duino_config_t tab5_config = TAB5DUINO_CONFIG_DEFAULT;
   tab5_config.auto_init_touch = true;
   Tab5duino.begin(&tab5_config);
   ```

4. **Memory allocation failures:**
   ```cpp
   // Ensure PSRAM is enabled
   tab5duino_config_t tab5_config = TAB5DUINO_CONFIG_DEFAULT;
   tab5_config.enable_psram = true;
   Tab5duino.begin(&tab5_config);
   ```

### Debug Output

Enable detailed logging:
```cpp
// In lv_conf.h
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_INFO

// View performance in real-time
#define LV_USE_PERF_MONITOR 1
#define LV_USE_MEM_MONITOR 1
```

## Advanced Features

### Custom Display Configuration

```cpp
// Access underlying HAL for custom configuration
lv_disp_t* disp = LVGL.getDisplay();
display_handle_t hal = /* get from display */;

// Custom brightness control
LVGL.setBrightness(128);

// Custom rotation
LVGL.setRotation(90);
```

### Multi-Touch Support

```cpp
// Multi-touch is enabled by default
// Access raw touch data through HAL if needed
lv_indev_t* touch = LVGL.getTouchpad();
```

### Integration with Other Tab5duino Features

```cpp
// Combine with IMU for gesture control
#include "hal/imu_hal.h"

void setup() {
    Tab5duino.begin();
    LVGL.begin();
    LVGL.start();
    
    // IMU is also initialized automatically
    // Can be used for tilt-based UI interactions
}
```

This migration guide should help you transition smoothly to the Tab5duino LVGL integration while taking full advantage of the ESP32-P4 hardware capabilities.