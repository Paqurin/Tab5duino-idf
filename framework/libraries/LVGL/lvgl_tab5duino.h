/**
 * @file lvgl_tab5duino.h
 * @brief LVGL Integration Layer for Tab5duino Framework
 * 
 * This file provides seamless integration between LVGL and the Tab5duino framework,
 * including display driver, touch input, and memory management optimizations.
 * 
 * Hardware Configuration:
 * - Display: 1280x720 MIPI-DSI with PPA acceleration
 * - Touch: GT911 multi-touch controller  
 * - Memory: 32MB PSRAM for frame buffers
 * - Target: 60fps performance
 */

#ifndef _LVGL_TAB5DUINO_H_
#define _LVGL_TAB5DUINO_H_

#include "Tab5duino.h"
#include "hal/display_hal.h"
#include "hal/touch_hal.h"
#include "lvgl.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// LVGL Integration Configuration
#define LVGL_TAB5_BUFFER_COUNT          2       // Double buffering
#define LVGL_TAB5_BUFFER_SIZE_LINES     120     // Lines per buffer (optimized for memory)
#define LVGL_TAB5_TASK_PRIORITY         2       // LVGL task priority
#define LVGL_TAB5_TASK_STACK_SIZE       (8 * 1024)  // 8KB stack
#define LVGL_TAB5_TASK_CORE             1       // Run on core 1
#define LVGL_TAB5_TICK_PERIOD_MS        5       // 5ms tick for smooth animations

// Performance optimization settings
#define LVGL_TAB5_USE_PPA_ACCELERATION  1       // Enable PPA hardware acceleration
#define LVGL_TAB5_USE_VSYNC             1       // Enable VSYNC for tear-free rendering
#define LVGL_TAB5_PREFETCH_LINES        4       // Lines to prefetch for PPA

// Memory allocation configuration
#define LVGL_TAB5_USE_PSRAM_BUFFERS     1       // Use PSRAM for frame buffers
#define LVGL_TAB5_PSRAM_CACHE_SIZE      (2 * 1024 * 1024)  // 2MB PSRAM cache

// Touch configuration
#define LVGL_TAB5_TOUCH_THRESHOLD       10      // Minimum movement for touch drag
#define LVGL_TAB5_GESTURE_THRESHOLD     50      // Minimum distance for gesture recognition

// LVGL Tab5duino integration handle
typedef struct lvgl_tab5_handle_s* lvgl_tab5_handle_t;

// Configuration structure
typedef struct {
    // Display configuration
    bool enable_ppa_acceleration;
    bool enable_vsync;
    bool enable_double_buffer;
    uint16_t buffer_lines;
    uint8_t rotation;
    
    // Touch configuration  
    bool enable_multi_touch;
    bool enable_gestures;
    uint8_t touch_threshold;
    uint16_t gesture_threshold;
    
    // Performance settings
    uint8_t task_priority;
    uint16_t task_stack_size;
    uint8_t task_core;
    uint8_t tick_period_ms;
    
    // Memory settings
    bool use_psram_buffers;
    uint32_t psram_cache_size;
} lvgl_tab5_config_t;

// LVGL display and input device handles (for advanced users)
typedef struct {
    lv_disp_t* display;
    lv_indev_t* touchpad;
    lv_disp_draw_buf_t* draw_buffer;
    lv_color_t* buffer1;
    lv_color_t* buffer2;
    display_handle_t display_hal;
    touch_handle_t touch_hal;
    TaskHandle_t lvgl_task;
    SemaphoreHandle_t lvgl_mutex;
    bool initialized;
} lvgl_tab5_context_t;

// Callback types
typedef void (*lvgl_ready_callback_t)(lvgl_tab5_handle_t handle, void* user_data);
typedef void (*lvgl_error_callback_t)(lvgl_tab5_handle_t handle, esp_err_t error, void* user_data);

/**
 * Initialize LVGL with Tab5duino framework integration
 * 
 * @param config Configuration structure (NULL for defaults)
 * @param handle Output handle for LVGL instance
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_init(const lvgl_tab5_config_t* config, lvgl_tab5_handle_t* handle);

/**
 * Start LVGL task and enable input processing
 * 
 * @param handle LVGL handle from lvgl_tab5_init()
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_start(lvgl_tab5_handle_t handle);

/**
 * Stop LVGL task and disable input processing
 * 
 * @param handle LVGL handle
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_stop(lvgl_tab5_handle_t handle);

/**
 * Deinitialize LVGL and free all resources
 * 
 * @param handle LVGL handle
 */
void lvgl_tab5_deinit(lvgl_tab5_handle_t handle);

/**
 * Lock LVGL for thread-safe operations
 * Use this before calling any LVGL functions from outside the LVGL task
 * 
 * @param handle LVGL handle
 * @param timeout_ms Timeout in milliseconds (portMAX_DELAY for infinite)
 * @return true if locked successfully
 */
bool lvgl_tab5_lock(lvgl_tab5_handle_t handle, uint32_t timeout_ms);

/**
 * Unlock LVGL after thread-safe operations
 * 
 * @param handle LVGL handle
 */
void lvgl_tab5_unlock(lvgl_tab5_handle_t handle);

/**
 * Force a full screen refresh
 * Useful after display configuration changes
 * 
 * @param handle LVGL handle
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_refresh_display(lvgl_tab5_handle_t handle);

/**
 * Get LVGL display object for advanced operations
 * 
 * @param handle LVGL handle
 * @return LVGL display object or NULL on error
 */
lv_disp_t* lvgl_tab5_get_display(lvgl_tab5_handle_t handle);

/**
 * Get LVGL input device object for advanced operations
 * 
 * @param handle LVGL handle  
 * @return LVGL input device object or NULL on error
 */
lv_indev_t* lvgl_tab5_get_touchpad(lvgl_tab5_handle_t handle);

/**
 * Register callback for LVGL ready event
 * 
 * @param handle LVGL handle
 * @param callback Callback function
 * @param user_data User data for callback
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_register_ready_callback(lvgl_tab5_handle_t handle,
                                           lvgl_ready_callback_t callback,
                                           void* user_data);

/**
 * Register callback for LVGL error events
 * 
 * @param handle LVGL handle
 * @param callback Callback function
 * @param user_data User data for callback
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_register_error_callback(lvgl_tab5_handle_t handle,
                                           lvgl_error_callback_t callback,
                                           void* user_data);

/**
 * Get performance statistics
 * 
 * @param handle LVGL handle
 * @param fps_avg Average FPS over last second
 * @param cpu_usage CPU usage percentage
 * @param mem_used Memory used in bytes
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_get_performance_stats(lvgl_tab5_handle_t handle,
                                         uint16_t* fps_avg,
                                         uint8_t* cpu_usage,
                                         uint32_t* mem_used);

/**
 * Set display brightness
 * 
 * @param handle LVGL handle
 * @param brightness Brightness level (0-255)
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_set_brightness(lvgl_tab5_handle_t handle, uint8_t brightness);

/**
 * Get display brightness
 * 
 * @param handle LVGL handle
 * @return Current brightness level (0-255) or 0 on error
 */
uint8_t lvgl_tab5_get_brightness(lvgl_tab5_handle_t handle);

/**
 * Set display rotation
 * 
 * @param handle LVGL handle
 * @param rotation Rotation angle (0, 90, 180, 270 degrees)
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_set_rotation(lvgl_tab5_handle_t handle, uint16_t rotation);

/**
 * Enable or disable PPA hardware acceleration
 * 
 * @param handle LVGL handle
 * @param enable true to enable, false to disable
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_set_ppa_acceleration(lvgl_tab5_handle_t handle, bool enable);

/**
 * Create a task-safe screen object
 * This function locks LVGL and creates a screen
 * 
 * @param handle LVGL handle
 * @return New screen object or NULL on error
 */
lv_obj_t* lvgl_tab5_create_screen(lvgl_tab5_handle_t handle);

/**
 * Load a screen safely
 * This function locks LVGL and loads the specified screen
 * 
 * @param handle LVGL handle
 * @param screen Screen object to load
 * @return ESP_OK on success
 */
esp_err_t lvgl_tab5_load_screen(lvgl_tab5_handle_t handle, lv_obj_t* screen);

/**
 * Check if LVGL is ready for operations
 * 
 * @param handle LVGL handle
 * @return true if ready, false otherwise
 */
bool lvgl_tab5_is_ready(lvgl_tab5_handle_t handle);

// Memory management functions (used by lv_conf.h)
void* tab5duino_lvgl_malloc(size_t size);
void* tab5duino_lvgl_realloc(void* ptr, size_t size);
void tab5duino_lvgl_free(void* ptr);

// Default configuration
extern const lvgl_tab5_config_t LVGL_TAB5_CONFIG_DEFAULT;

// Utility macros for common operations
#define LVGL_TAB5_CALL_SAFE(handle, func, ...) do { \
    if (lvgl_tab5_lock(handle, portMAX_DELAY)) { \
        func(__VA_ARGS__); \
        lvgl_tab5_unlock(handle); \
    } \
} while(0)

#define LVGL_TAB5_CALL_SAFE_RETURN(handle, func, ret_var, ...) do { \
    if (lvgl_tab5_lock(handle, portMAX_DELAY)) { \
        ret_var = func(__VA_ARGS__); \
        lvgl_tab5_unlock(handle); \
    } \
} while(0)

#ifdef __cplusplus
}

// C++ wrapper class
class LVGLTab5duino {
private:
    lvgl_tab5_handle_t handle_;
    bool initialized_;

public:
    LVGLTab5duino();
    ~LVGLTab5duino();

    // Framework integration
    esp_err_t begin(const lvgl_tab5_config_t* config = &LVGL_TAB5_CONFIG_DEFAULT);
    esp_err_t start();
    esp_err_t stop();
    void end();

    // Thread-safe operations
    bool lock(uint32_t timeout_ms = portMAX_DELAY);
    void unlock();
    
    // Display management
    lv_obj_t* createScreen();
    esp_err_t loadScreen(lv_obj_t* screen);
    esp_err_t refreshDisplay();
    
    // Configuration
    esp_err_t setBrightness(uint8_t brightness);
    uint8_t getBrightness();
    esp_err_t setRotation(uint16_t rotation);
    esp_err_t setPPAAcceleration(bool enable);
    
    // Information
    lv_disp_t* getDisplay();
    lv_indev_t* getTouchpad();
    bool isReady();
    esp_err_t getPerformanceStats(uint16_t* fps_avg, uint8_t* cpu_usage, uint32_t* mem_used);
    
    // Callback registration
    esp_err_t registerReadyCallback(lvgl_ready_callback_t callback, void* user_data);
    esp_err_t registerErrorCallback(lvgl_error_callback_t callback, void* user_data);
    
    // Utility functions
    template<typename Func, typename... Args>
    auto callSafe(Func&& func, Args&&... args) -> decltype(func(args...)) {
        if (lock()) {
            auto result = func(std::forward<Args>(args)...);
            unlock();
            return result;
        }
        return {};
    }
    
    template<typename Func, typename... Args>
    void callSafeVoid(Func&& func, Args&&... args) {
        if (lock()) {
            func(std::forward<Args>(args)...);
            unlock();
        }
    }
};

// Global instance (similar to Tab5duino framework)
extern LVGLTab5duino& LVGL;

#endif // __cplusplus

#endif // _LVGL_TAB5DUINO_H_