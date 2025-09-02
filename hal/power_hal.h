/*
 * Power HAL for M5Stack Tab5
 * Solar Charging and Power Management System
 */

#ifndef _POWER_HAL_H_
#define _POWER_HAL_H_

#include "esp_err.h"
#include "esp_pm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Power modes
typedef enum {
    POWER_MODE_PERFORMANCE = 0,  // Maximum performance
    POWER_MODE_BALANCED,         // Balanced performance/power
    POWER_MODE_POWER_SAVE,       // Power saving mode
    POWER_MODE_DEEP_SLEEP,       // Deep sleep mode
    POWER_MODE_HIBERNATION,      // Hibernation mode
    POWER_MODE_EMERGENCY         // Emergency power mode
} power_mode_t;

// Battery information
typedef struct {
    float voltage_v;            // Battery voltage in volts
    float current_ma;           // Battery current in mA (+ charging, - discharging)
    uint8_t percentage;         // Battery percentage (0-100)
    float temperature_c;        // Battery temperature in Celsius
    bool is_charging;           // Charging status
    bool is_full;               // Full charge status
    bool is_low;                // Low battery warning
    bool is_critical;           // Critical battery warning
    uint32_t cycle_count;       // Battery charge cycles
    uint32_t time_to_empty_min; // Estimated time to empty (minutes)
    uint32_t time_to_full_min;  // Estimated time to full charge (minutes)
} power_battery_info_t;

// Solar panel information
typedef struct {
    float voltage_v;            // Solar panel voltage
    float current_ma;           // Solar panel current
    float power_mw;             // Solar panel power in mW
    bool is_active;             // Solar panel active status
    float efficiency;           // Current efficiency (0.0-1.0)
    uint32_t total_energy_wh;   // Total energy harvested (Wh)
} power_solar_info_t;

// Power consumption information
typedef struct {
    float cpu_power_mw;         // CPU power consumption
    float display_power_mw;     // Display power consumption
    float wifi_power_mw;        // WiFi power consumption
    float bluetooth_power_mw;   // Bluetooth power consumption
    float sensors_power_mw;     // Sensors power consumption
    float total_power_mw;       // Total power consumption
    float avg_power_1min_mw;    // 1-minute average power
    float avg_power_5min_mw;    // 5-minute average power
    float avg_power_15min_mw;   // 15-minute average power
} power_consumption_t;

// Power events
typedef enum {
    POWER_EVENT_NONE = 0,
    POWER_EVENT_BATTERY_LOW,
    POWER_EVENT_BATTERY_CRITICAL,
    POWER_EVENT_BATTERY_FULL,
    POWER_EVENT_CHARGING_START,
    POWER_EVENT_CHARGING_STOP,
    POWER_EVENT_SOLAR_ACTIVE,
    POWER_EVENT_SOLAR_INACTIVE,
    POWER_EVENT_OVERVOLTAGE,
    POWER_EVENT_UNDERVOLTAGE,
    POWER_EVENT_OVERTEMPERATURE,
    POWER_EVENT_POWER_MODE_CHANGE,
    POWER_EVENT_SHUTDOWN_REQUEST,
    POWER_EVENT_WAKEUP
} power_event_type_t;

// Power event data
typedef struct {
    power_event_type_t type;
    uint32_t data;              // Event-specific data
    uint64_t timestamp_us;      // Event timestamp
} power_event_t;

// Power configuration
typedef struct {
    // Battery configuration
    float battery_capacity_mah;     // Battery capacity in mAh
    float battery_min_voltage;      // Minimum safe voltage
    float battery_max_voltage;      // Maximum safe voltage
    float battery_low_threshold;    // Low battery threshold (%)
    float battery_critical_threshold; // Critical battery threshold (%)
    
    // Solar configuration
    bool enable_solar;              // Enable solar charging
    float solar_max_voltage;        // Maximum solar input voltage
    float solar_max_current;        // Maximum solar input current
    
    // Power management
    power_mode_t default_mode;      // Default power mode
    uint32_t auto_sleep_timeout_s;  // Auto sleep timeout (seconds)
    uint32_t deep_sleep_timeout_s;  // Deep sleep timeout (seconds)
    bool enable_auto_power_mgmt;    // Enable automatic power management
    
    // Monitoring
    uint32_t monitor_interval_ms;   // Power monitoring interval
    bool enable_power_logging;      // Enable power consumption logging
} power_config_t;

// Power handle
typedef struct power_handle_s* power_handle_t;

// Power callback types
typedef void (*power_event_callback_t)(power_handle_t handle,
                                      const power_event_t* event,
                                      void* user_data);

typedef void (*power_monitor_callback_t)(power_handle_t handle,
                                        const power_battery_info_t* battery,
                                        const power_solar_info_t* solar,
                                        const power_consumption_t* consumption,
                                        void* user_data);

// Power initialization and management
esp_err_t power_hal_init(const power_config_t* config, power_handle_t* handle);
esp_err_t power_hal_deinit(power_handle_t handle);
esp_err_t power_hal_start_monitoring(power_handle_t handle);
esp_err_t power_hal_stop_monitoring(power_handle_t handle);

// Power mode management
esp_err_t power_hal_set_mode(power_handle_t handle, power_mode_t mode);
power_mode_t power_hal_get_mode(power_handle_t handle);
esp_err_t power_hal_request_mode_change(power_handle_t handle, power_mode_t mode, uint32_t duration_s);

// Sleep and wake functions
esp_err_t power_hal_enter_light_sleep(power_handle_t handle, uint32_t sleep_time_ms);
esp_err_t power_hal_enter_deep_sleep(power_handle_t handle, uint32_t sleep_time_s);
esp_err_t power_hal_enter_hibernation(power_handle_t handle);
esp_err_t power_hal_configure_wakeup_source(power_handle_t handle, uint32_t wakeup_mask);

// Battery information
esp_err_t power_hal_get_battery_info(power_handle_t handle, power_battery_info_t* info);
esp_err_t power_hal_get_battery_voltage(power_handle_t handle, float* voltage);
esp_err_t power_hal_get_battery_percentage(power_handle_t handle, uint8_t* percentage);
bool power_hal_is_battery_charging(power_handle_t handle);
bool power_hal_is_battery_low(power_handle_t handle);

// Solar panel information
esp_err_t power_hal_get_solar_info(power_handle_t handle, power_solar_info_t* info);
esp_err_t power_hal_get_solar_power(power_handle_t handle, float* power_mw);
bool power_hal_is_solar_active(power_handle_t handle);

// Power consumption monitoring
esp_err_t power_hal_get_consumption(power_handle_t handle, power_consumption_t* consumption);
esp_err_t power_hal_get_total_power(power_handle_t handle, float* power_mw);
esp_err_t power_hal_estimate_runtime(power_handle_t handle, uint32_t* minutes);

// CPU frequency and voltage scaling
esp_err_t power_hal_set_cpu_frequency(power_handle_t handle, uint32_t freq_mhz);
uint32_t power_hal_get_cpu_frequency(power_handle_t handle);
esp_err_t power_hal_enable_dvfs(power_handle_t handle, bool enable);

// Peripheral power control
esp_err_t power_hal_set_display_power(power_handle_t handle, bool enable);
esp_err_t power_hal_set_wifi_power(power_handle_t handle, bool enable);
esp_err_t power_hal_set_bluetooth_power(power_handle_t handle, bool enable);
esp_err_t power_hal_set_sensor_power(power_handle_t handle, bool enable);

// Charging control
esp_err_t power_hal_set_charging_current(power_handle_t handle, float current_ma);
float power_hal_get_charging_current(power_handle_t handle);
esp_err_t power_hal_enable_charging(power_handle_t handle, bool enable);

// Power limits and protection
esp_err_t power_hal_set_power_limit(power_handle_t handle, float limit_mw);
float power_hal_get_power_limit(power_handle_t handle);
esp_err_t power_hal_enable_ovp_protection(power_handle_t handle, bool enable);
esp_err_t power_hal_enable_ocp_protection(power_handle_t handle, bool enable);

// Callback registration
esp_err_t power_hal_register_event_callback(power_handle_t handle,
                                           power_event_callback_t callback,
                                           void* user_data);
esp_err_t power_hal_register_monitor_callback(power_handle_t handle,
                                             power_monitor_callback_t callback,
                                             void* user_data);

// Emergency functions
esp_err_t power_hal_emergency_shutdown(power_handle_t handle);
esp_err_t power_hal_emergency_mode(power_handle_t handle, bool enable);
bool power_hal_is_emergency_mode(power_handle_t handle);

// Information functions
esp_err_t power_hal_get_info(power_handle_t handle, power_config_t* info);
bool power_hal_is_ready(power_handle_t handle);

// Utility functions
static inline float power_voltage_to_percentage(float voltage, float min_v, float max_v) {
    if (voltage <= min_v) return 0.0f;
    if (voltage >= max_v) return 100.0f;
    return ((voltage - min_v) / (max_v - min_v)) * 100.0f;
}

static inline uint32_t power_estimate_runtime(float battery_capacity_mah, float current_draw_ma) {
    if (current_draw_ma <= 0.0f) return UINT32_MAX;  // Infinite runtime
    return (uint32_t)((battery_capacity_mah / current_draw_ma) * 60.0f);  // Minutes
}

static inline float power_calculate_efficiency(float input_power_mw, float output_power_mw) {
    if (input_power_mw <= 0.0f) return 0.0f;
    return output_power_mw / input_power_mw;
}

// Wake-up source definitions
#define POWER_WAKEUP_TIMER          (1 << 0)
#define POWER_WAKEUP_TOUCHPAD       (1 << 1)
#define POWER_WAKEUP_GPIO           (1 << 2)
#define POWER_WAKEUP_UART           (1 << 3)
#define POWER_WAKEUP_IMU            (1 << 4)
#define POWER_WAKEUP_USB            (1 << 5)
#define POWER_WAKEUP_SOLAR          (1 << 6)
#define POWER_WAKEUP_EMERGENCY      (1 << 7)

// Default configuration
extern const power_config_t POWER_CONFIG_DEFAULT;

#ifdef __cplusplus
}
#endif

#endif // _POWER_HAL_H_