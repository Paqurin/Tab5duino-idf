/*
 * Tab5duino Framework for ESP32-P4 M5Stack Tab5
 * Arduino-compatible framework built on ESP-IDF foundation
 * 
 * Copyright (c) 2024 M5Stack
 * Licensed under MIT License
 */

#ifndef _TAB5DUINO_H_
#define _TAB5DUINO_H_

#include "esp_idf_version.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Framework version
#define TAB5DUINO_VERSION_MAJOR 1
#define TAB5DUINO_VERSION_MINOR 0
#define TAB5DUINO_VERSION_PATCH 0

// Hardware specifications
#define TAB5_DISPLAY_WIDTH  1280
#define TAB5_DISPLAY_HEIGHT 720
#define TAB5_PSRAM_SIZE     (32 * 1024 * 1024)  // 32MB PSRAM
#define TAB5_FLASH_SIZE     (16 * 1024 * 1024)  // 16MB Flash

// Core subsystem states
typedef enum {
    TAB5_SUBSYSTEM_UNINITIALIZED = 0,
    TAB5_SUBSYSTEM_INITIALIZING,
    TAB5_SUBSYSTEM_READY,
    TAB5_SUBSYSTEM_ERROR
} tab5_subsystem_state_t;

// Hardware subsystems
typedef enum {
    TAB5_SUBSYSTEM_DISPLAY = 0,
    TAB5_SUBSYSTEM_TOUCH,
    TAB5_SUBSYSTEM_IMU,
    TAB5_SUBSYSTEM_AUDIO,
    TAB5_SUBSYSTEM_POWER,
    TAB5_SUBSYSTEM_USB,
    TAB5_SUBSYSTEM_WIFI,
    TAB5_SUBSYSTEM_LVGL,
    TAB5_SUBSYSTEM_COUNT
} tab5_subsystem_t;

// Framework configuration structure
typedef struct {
    bool auto_init_display;
    bool auto_init_touch;
    bool auto_init_imu;
    bool auto_init_audio;
    bool auto_init_lvgl;
    bool enable_psram;
    bool enable_usb_serial;
    uint32_t loop_stack_size;
    uint8_t loop_task_priority;
    uint8_t loop_task_core;
} tab5duino_config_t;

// Framework instance structure
typedef struct {
    tab5duino_config_t config;
    tab5_subsystem_state_t subsystem_states[TAB5_SUBSYSTEM_COUNT];
    TaskHandle_t loop_task_handle;
    bool framework_initialized;
    bool user_setup_called;
    uint64_t boot_time_us;
} tab5duino_instance_t;

// Core framework functions
esp_err_t tab5duino_init(const tab5duino_config_t* config);
esp_err_t tab5duino_start(void);
esp_err_t tab5duino_stop(void);
void tab5duino_deinit(void);

// Subsystem management
esp_err_t tab5duino_init_subsystem(tab5_subsystem_t subsystem);
esp_err_t tab5duino_deinit_subsystem(tab5_subsystem_t subsystem);
tab5_subsystem_state_t tab5duino_get_subsystem_state(tab5_subsystem_t subsystem);

// Framework information
const char* tab5duino_get_version_string(void);
uint64_t tab5duino_get_boot_time_us(void);
bool tab5duino_is_ready(void);

// User application interface (implemented by user)
void setup(void);
void loop(void);

// Optional user hooks
void __attribute__((weak)) on_framework_init(void);
void __attribute__((weak)) on_framework_ready(void);
void __attribute__((weak)) on_subsystem_error(tab5_subsystem_t subsystem, esp_err_t error);
void __attribute__((weak)) on_lvgl_ready(void);

// Memory management helpers
void* tab5duino_malloc(size_t size);
void* tab5duino_calloc(size_t num, size_t size);
void* tab5duino_realloc(void* ptr, size_t size);
void tab5duino_free(void* ptr);

// Default configuration
extern const tab5duino_config_t TAB5DUINO_CONFIG_DEFAULT;

#ifdef __cplusplus
}

// C++ Framework Class
class Tab5duinoFramework {
private:
    static Tab5duinoFramework* instance;
    bool initialized;

public:
    Tab5duinoFramework();
    ~Tab5duinoFramework();

    // Singleton access
    static Tab5duinoFramework& getInstance();

    // Framework lifecycle
    esp_err_t begin(const tab5duino_config_t* config = &TAB5DUINO_CONFIG_DEFAULT);
    esp_err_t start();
    void end();

    // Subsystem management
    esp_err_t initSubsystem(tab5_subsystem_t subsystem);
    esp_err_t deinitSubsystem(tab5_subsystem_t subsystem);
    tab5_subsystem_state_t getSubsystemState(tab5_subsystem_t subsystem);

    // Information
    const char* getVersionString();
    uint64_t getBootTimeUs();
    bool isReady();

    // Memory management
    void* malloc(size_t size);
    void* calloc(size_t num, size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);
};

// Global framework instance
extern Tab5duinoFramework& Tab5duino;

#endif // __cplusplus

#endif // _TAB5DUINO_H_