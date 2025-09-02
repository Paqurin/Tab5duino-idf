/*
 * IMU HAL for M5Stack Tab5
 * BMI270 6-axis IMU with Motion Detection and Gesture Recognition
 */

#ifndef _IMU_HAL_H_
#define _IMU_HAL_H_

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// IMU sensor ranges and resolutions
#define IMU_ACCEL_RANGE_2G      2
#define IMU_ACCEL_RANGE_4G      4
#define IMU_ACCEL_RANGE_8G      8
#define IMU_ACCEL_RANGE_16G     16

#define IMU_GYRO_RANGE_250DPS   250
#define IMU_GYRO_RANGE_500DPS   500
#define IMU_GYRO_RANGE_1000DPS  1000
#define IMU_GYRO_RANGE_2000DPS  2000

// IMU data rates
typedef enum {
    IMU_ODR_25HZ = 25,
    IMU_ODR_50HZ = 50,
    IMU_ODR_100HZ = 100,
    IMU_ODR_200HZ = 200,
    IMU_ODR_400HZ = 400,
    IMU_ODR_800HZ = 800,
    IMU_ODR_1600HZ = 1600
} imu_odr_t;

// IMU sensor data structure
typedef struct {
    float x;    // X-axis value
    float y;    // Y-axis value
    float z;    // Z-axis value
    uint64_t timestamp_us;  // Timestamp in microseconds
    bool is_valid;          // Data validity flag
} imu_data_t;

// IMU orientation
typedef enum {
    IMU_ORIENTATION_PORTRAIT = 0,
    IMU_ORIENTATION_LANDSCAPE_LEFT,
    IMU_ORIENTATION_PORTRAIT_INVERTED,
    IMU_ORIENTATION_LANDSCAPE_RIGHT,
    IMU_ORIENTATION_FACE_UP,
    IMU_ORIENTATION_FACE_DOWN,
    IMU_ORIENTATION_UNKNOWN
} imu_orientation_t;

// IMU motion events
typedef enum {
    IMU_MOTION_NONE = 0,
    IMU_MOTION_SINGLE_TAP,
    IMU_MOTION_DOUBLE_TAP,
    IMU_MOTION_SIGNIFICANT_MOTION,
    IMU_MOTION_STEP_DETECTED,
    IMU_MOTION_TILT,
    IMU_MOTION_PICKUP,
    IMU_MOTION_GLANCE,
    IMU_MOTION_WAKEUP,
    IMU_MOTION_NO_MOTION,
    IMU_MOTION_ANY_MOTION,
    IMU_MOTION_ORIENTATION_CHANGE,
    IMU_MOTION_HIGH_G,
    IMU_MOTION_LOW_G,
    IMU_MOTION_FLAT,
    IMU_MOTION_SHAKE
} imu_motion_event_t;

// IMU gesture data
typedef struct {
    imu_motion_event_t event;
    imu_orientation_t orientation;
    float intensity;        // Motion intensity (0.0 - 1.0)
    uint32_t duration_ms;   // Event duration
    uint64_t timestamp_us;  // Event timestamp
} imu_gesture_t;

// IMU activity data
typedef struct {
    uint32_t step_count;
    float activity_level;   // 0.0 (stationary) to 1.0 (very active)
    bool is_walking;
    bool is_running;
    bool is_stationary;
    uint32_t active_time_ms;
    uint32_t stationary_time_ms;
} imu_activity_t;

// IMU configuration
typedef struct {
    uint8_t i2c_address;          // BMI270 I2C address (0x68 or 0x69)
    uint8_t sda_pin;              // I2C SDA pin
    uint8_t scl_pin;              // I2C SCL pin
    uint8_t int1_pin;             // Interrupt 1 pin
    uint8_t int2_pin;             // Interrupt 2 pin
    uint32_t i2c_frequency;       // I2C clock frequency
    
    // Accelerometer configuration
    uint8_t accel_range;          // Accelerometer range (2G, 4G, 8G, 16G)
    imu_odr_t accel_odr;          // Accelerometer output data rate
    bool accel_enable;
    
    // Gyroscope configuration
    uint16_t gyro_range;          // Gyroscope range (250, 500, 1000, 2000 DPS)
    imu_odr_t gyro_odr;           // Gyroscope output data rate
    bool gyro_enable;
    
    // Motion detection configuration
    bool enable_motion_detection;
    bool enable_step_counter;
    bool enable_orientation_detection;
    bool enable_tap_detection;
    bool enable_gesture_detection;
    
    // Thresholds
    float motion_threshold;       // Motion detection threshold (g)
    float tap_threshold;          // Tap detection threshold (g)
    uint16_t tap_duration_ms;     // Maximum tap duration
    uint16_t double_tap_window_ms; // Double tap time window
    float tilt_threshold;         // Tilt detection threshold (degrees)
} imu_config_t;

// IMU handle
typedef struct imu_handle_s* imu_handle_t;

// IMU callback types
typedef void (*imu_data_callback_t)(imu_handle_t handle,
                                   const imu_data_t* accel,
                                   const imu_data_t* gyro,
                                   void* user_data);

typedef void (*imu_motion_callback_t)(imu_handle_t handle,
                                     const imu_gesture_t* gesture,
                                     void* user_data);

typedef void (*imu_orientation_callback_t)(imu_handle_t handle,
                                          imu_orientation_t orientation,
                                          void* user_data);

// IMU initialization and management
esp_err_t imu_hal_init(const imu_config_t* config, imu_handle_t* handle);
esp_err_t imu_hal_deinit(imu_handle_t handle);
esp_err_t imu_hal_start(imu_handle_t handle);
esp_err_t imu_hal_stop(imu_handle_t handle);
esp_err_t imu_hal_reset(imu_handle_t handle);

// IMU data reading
esp_err_t imu_hal_read_accel(imu_handle_t handle, imu_data_t* accel_data);
esp_err_t imu_hal_read_gyro(imu_handle_t handle, imu_data_t* gyro_data);
esp_err_t imu_hal_read_both(imu_handle_t handle, imu_data_t* accel_data, imu_data_t* gyro_data);

// Motion and orientation
esp_err_t imu_hal_get_orientation(imu_handle_t handle, imu_orientation_t* orientation);
esp_err_t imu_hal_get_gesture(imu_handle_t handle, imu_gesture_t* gesture);
esp_err_t imu_hal_get_activity(imu_handle_t handle, imu_activity_t* activity);

// Temperature reading
esp_err_t imu_hal_read_temperature(imu_handle_t handle, float* temperature);

// Calibration
esp_err_t imu_hal_calibrate_accel(imu_handle_t handle);
esp_err_t imu_hal_calibrate_gyro(imu_handle_t handle);
esp_err_t imu_hal_set_accel_offset(imu_handle_t handle, float x_offset, float y_offset, float z_offset);
esp_err_t imu_hal_set_gyro_offset(imu_handle_t handle, float x_offset, float y_offset, float z_offset);

// Configuration
esp_err_t imu_hal_set_accel_range(imu_handle_t handle, uint8_t range);
esp_err_t imu_hal_set_gyro_range(imu_handle_t handle, uint16_t range);
esp_err_t imu_hal_set_accel_odr(imu_handle_t handle, imu_odr_t odr);
esp_err_t imu_hal_set_gyro_odr(imu_handle_t handle, imu_odr_t odr);

// Motion detection configuration
esp_err_t imu_hal_enable_motion_detection(imu_handle_t handle, bool enable);
esp_err_t imu_hal_set_motion_threshold(imu_handle_t handle, float threshold);
esp_err_t imu_hal_enable_step_counter(imu_handle_t handle, bool enable);
esp_err_t imu_hal_reset_step_counter(imu_handle_t handle);

// Interrupt configuration
esp_err_t imu_hal_enable_interrupt(imu_handle_t handle, imu_motion_event_t event, bool enable);
esp_err_t imu_hal_clear_interrupt(imu_handle_t handle);

// Callback registration
esp_err_t imu_hal_register_data_callback(imu_handle_t handle,
                                        imu_data_callback_t callback,
                                        void* user_data);
esp_err_t imu_hal_register_motion_callback(imu_handle_t handle,
                                          imu_motion_callback_t callback,
                                          void* user_data);
esp_err_t imu_hal_register_orientation_callback(imu_handle_t handle,
                                               imu_orientation_callback_t callback,
                                               void* user_data);

// Information functions
esp_err_t imu_hal_get_info(imu_handle_t handle, imu_config_t* info);
esp_err_t imu_hal_get_chip_id(imu_handle_t handle, uint8_t* chip_id);
esp_err_t imu_hal_get_firmware_version(imu_handle_t handle, uint16_t* version);
bool imu_hal_is_ready(imu_handle_t handle);

// Utility functions
static inline float imu_accel_to_ms2(float accel_g) {
    return accel_g * 9.80665f;  // Convert g to m/s²
}

static inline float imu_gyro_to_radps(float gyro_dps) {
    return gyro_dps * M_PI / 180.0f;  // Convert degrees/s to radians/s
}

static inline float imu_magnitude(const imu_data_t* data) {
    if (!data) return 0.0f;
    return sqrtf(data->x * data->x + data->y * data->y + data->z * data->z);
}

static inline float imu_angle_between(const imu_data_t* a, const imu_data_t* b) {
    if (!a || !b) return 0.0f;
    float dot = a->x * b->x + a->y * b->y + a->z * b->z;
    float mag_a = imu_magnitude(a);
    float mag_b = imu_magnitude(b);
    if (mag_a == 0.0f || mag_b == 0.0f) return 0.0f;
    return acosf(dot / (mag_a * mag_b)) * 180.0f / M_PI;
}

// Default configuration
extern const imu_config_t IMU_CONFIG_DEFAULT;

#ifdef __cplusplus
}
#endif

#endif // _IMU_HAL_H_