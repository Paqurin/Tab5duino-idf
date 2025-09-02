/*
 * Touch HAL for M5Stack Tab5
 * GT911 Capacitive Touch Controller with Multi-touch Support
 */

#ifndef _TOUCH_HAL_H_
#define _TOUCH_HAL_H_

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Touch specifications
#define TOUCH_MAX_POINTS        10    // GT911 supports up to 10 touch points
#define TOUCH_COORD_MAX_X       1280  // Match display resolution
#define TOUCH_COORD_MAX_Y       720
#define TOUCH_PRESSURE_MAX      255

// Touch point structure
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t pressure;     // 0-255, 0 means no touch
    uint8_t size;         // Touch area size
    uint8_t id;           // Touch point ID for tracking
    bool is_valid;        // Whether this point is valid
} touch_point_t;

// Touch event types
typedef enum {
    TOUCH_EVENT_NONE = 0,
    TOUCH_EVENT_PRESS,
    TOUCH_EVENT_RELEASE,
    TOUCH_EVENT_MOVE,
    TOUCH_EVENT_LONG_PRESS,
    TOUCH_EVENT_SWIPE_UP,
    TOUCH_EVENT_SWIPE_DOWN,
    TOUCH_EVENT_SWIPE_LEFT,
    TOUCH_EVENT_SWIPE_RIGHT,
    TOUCH_EVENT_PINCH_IN,
    TOUCH_EVENT_PINCH_OUT,
    TOUCH_EVENT_ROTATE
} touch_event_type_t;

// Touch gesture data
typedef struct {
    touch_event_type_t type;
    touch_point_t start_point;
    touch_point_t current_point;
    touch_point_t end_point;
    uint32_t duration_ms;
    int16_t distance_x;
    int16_t distance_y;
    float velocity;
    float angle;          // For rotation gestures (degrees)
    float scale;          // For pinch gestures
} touch_gesture_t;

// Touch configuration
typedef struct {
    uint8_t i2c_address;          // GT911 I2C address (0x5D or 0x14)
    uint8_t interrupt_pin;        // Interrupt GPIO pin
    uint8_t reset_pin;            // Reset GPIO pin
    uint8_t sda_pin;              // I2C SDA pin
    uint8_t scl_pin;              // I2C SCL pin
    uint32_t i2c_frequency;       // I2C clock frequency
    bool enable_multi_touch;      // Enable multi-touch support
    bool enable_gestures;         // Enable gesture recognition
    uint16_t debounce_ms;         // Touch debounce time
    uint8_t sensitivity;          // Touch sensitivity (0-255)
    bool flip_x;                  // Flip X coordinate
    bool flip_y;                  // Flip Y coordinate
    bool swap_xy;                 // Swap X and Y coordinates
} touch_config_t;

// Touch handle
typedef struct touch_handle_s* touch_handle_t;

// Touch callback types
typedef void (*touch_event_callback_t)(touch_handle_t handle, 
                                      const touch_point_t* points, 
                                      uint8_t point_count, 
                                      void* user_data);

typedef void (*touch_gesture_callback_t)(touch_handle_t handle,
                                        const touch_gesture_t* gesture,
                                        void* user_data);

// Touch initialization and management
esp_err_t touch_hal_init(const touch_config_t* config, touch_handle_t* handle);
esp_err_t touch_hal_deinit(touch_handle_t handle);
esp_err_t touch_hal_start(touch_handle_t handle);
esp_err_t touch_hal_stop(touch_handle_t handle);

// Touch reading functions
esp_err_t touch_hal_read_points(touch_handle_t handle, touch_point_t* points, uint8_t* count);
esp_err_t touch_hal_is_touched(touch_handle_t handle, bool* touched);
esp_err_t touch_hal_get_gesture(touch_handle_t handle, touch_gesture_t* gesture);

// Touch configuration
esp_err_t touch_hal_set_sensitivity(touch_handle_t handle, uint8_t sensitivity);
uint8_t touch_hal_get_sensitivity(touch_handle_t handle);
esp_err_t touch_hal_set_coordinate_transform(touch_handle_t handle, 
                                           bool flip_x, bool flip_y, bool swap_xy);
esp_err_t touch_hal_calibrate(touch_handle_t handle);

// Callback registration
esp_err_t touch_hal_register_event_callback(touch_handle_t handle,
                                          touch_event_callback_t callback,
                                          void* user_data);
esp_err_t touch_hal_register_gesture_callback(touch_handle_t handle,
                                            touch_gesture_callback_t callback,
                                            void* user_data);

// Information functions
esp_err_t touch_hal_get_info(touch_handle_t handle, touch_config_t* info);
esp_err_t touch_hal_get_firmware_version(touch_handle_t handle, uint16_t* version);
bool touch_hal_is_ready(touch_handle_t handle);

// Utility functions
static inline bool touch_point_is_valid(const touch_point_t* point) {
    return point && point->is_valid && point->pressure > 0;
}

static inline uint16_t touch_distance(const touch_point_t* p1, const touch_point_t* p2) {
    if (!p1 || !p2) return 0;
    int32_t dx = p1->x - p2->x;
    int32_t dy = p1->y - p2->y;
    return (uint16_t)sqrt(dx * dx + dy * dy);
}

static inline float touch_angle(const touch_point_t* p1, const touch_point_t* p2) {
    if (!p1 || !p2) return 0.0f;
    int32_t dx = p2->x - p1->x;
    int32_t dy = p2->y - p1->y;
    return atan2f(dy, dx) * 180.0f / M_PI;
}

// Default configuration
extern const touch_config_t TOUCH_CONFIG_DEFAULT;

#ifdef __cplusplus
}
#endif

#endif // _TOUCH_HAL_H_