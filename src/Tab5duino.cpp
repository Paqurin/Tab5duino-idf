/*
 * Tab5duino Framework Implementation
 * Arduino-compatible framework built on ESP-IDF foundation
 */

#include "Tab5duino.h"
#include "Arduino.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_psram.h"
#include "driver/usb_serial_jtag.h"
#include "lvgl_tab5duino.h"

static const char* TAG = "Tab5duino";

// Global framework instance
static tab5duino_instance_t g_tab5duino_instance = {0};

// Default configuration
const tab5duino_config_t TAB5DUINO_CONFIG_DEFAULT = {
    .auto_init_display = true,
    .auto_init_touch = true,
    .auto_init_imu = true,
    .auto_init_audio = false,  // Audio init on demand
    .auto_init_lvgl = true,   // Auto-init LVGL by default
    .enable_psram = true,
    .enable_usb_serial = true,
    .loop_stack_size = 8192,
    .loop_task_priority = 1,
    .loop_task_core = 1
};

// Subsystem names for logging
static const char* subsystem_names[] = {
    "Display", "Touch", "IMU", "Audio", "Power", "USB", "WiFi", "LVGL"
};

// Forward declarations
static void loop_task(void* parameter);
static esp_err_t init_hardware_subsystems(void);
static void deinit_hardware_subsystems(void);

// Framework initialization
esp_err_t tab5duino_init(const tab5duino_config_t* config) {
    if (g_tab5duino_instance.framework_initialized) {
        ESP_LOGW(TAG, "Framework already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing Tab5duino Framework v%d.%d.%d", 
             TAB5DUINO_VERSION_MAJOR, TAB5DUINO_VERSION_MINOR, TAB5DUINO_VERSION_PATCH);

    // Copy configuration
    if (config) {
        memcpy(&g_tab5duino_instance.config, config, sizeof(tab5duino_config_t));
    } else {
        memcpy(&g_tab5duino_instance.config, &TAB5DUINO_CONFIG_DEFAULT, sizeof(tab5duino_config_t));
    }

    // Record boot time
    g_tab5duino_instance.boot_time_us = esp_timer_get_time();

    // Initialize all subsystem states
    for (int i = 0; i < TAB5_SUBSYSTEM_COUNT; i++) {
        g_tab5duino_instance.subsystem_states[i] = TAB5_SUBSYSTEM_UNINITIALIZED;
    }

    // Initialize PSRAM if enabled
    if (g_tab5duino_instance.config.enable_psram) {
        if (esp_psram_is_initialized()) {
            ESP_LOGI(TAG, "PSRAM initialized: %d MB", esp_psram_get_size() / (1024 * 1024));
        } else {
            ESP_LOGW(TAG, "PSRAM not available");
        }
    }

    // Initialize USB Serial if enabled
    if (g_tab5duino_instance.config.enable_usb_serial) {
        usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
            .tx_buffer_size = 256,
            .rx_buffer_size = 256,
        };
        esp_err_t ret = usb_serial_jtag_driver_install(&usb_serial_jtag_config);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to initialize USB Serial: %s", esp_err_to_name(ret));
        }
    }

    // Initialize hardware subsystems
    esp_err_t ret = init_hardware_subsystems();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize hardware subsystems: %s", esp_err_to_name(ret));
        return ret;
    }

    g_tab5duino_instance.framework_initialized = true;

    // Call user init hook if available
    if (on_framework_init) {
        on_framework_init();
    }

    ESP_LOGI(TAG, "Framework initialization complete");
    return ESP_OK;
}

// Start the framework (create loop task)
esp_err_t tab5duino_start(void) {
    if (!g_tab5duino_instance.framework_initialized) {
        ESP_LOGE(TAG, "Framework not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (g_tab5duino_instance.loop_task_handle) {
        ESP_LOGW(TAG, "Framework already started");
        return ESP_OK;
    }

    // Create the main loop task
    BaseType_t result = xTaskCreatePinnedToCore(
        loop_task,
        "tab5duino_loop",
        g_tab5duino_instance.config.loop_stack_size,
        NULL,
        g_tab5duino_instance.config.loop_task_priority,
        &g_tab5duino_instance.loop_task_handle,
        g_tab5duino_instance.config.loop_task_core
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create loop task");
        return ESP_ERR_NO_MEM;
    }

    // Call user ready hook if available
    if (on_framework_ready) {
        on_framework_ready();
    }

    ESP_LOGI(TAG, "Framework started");
    return ESP_OK;
}

// Stop the framework
esp_err_t tab5duino_stop(void) {
    if (g_tab5duino_instance.loop_task_handle) {
        vTaskDelete(g_tab5duino_instance.loop_task_handle);
        g_tab5duino_instance.loop_task_handle = NULL;
        ESP_LOGI(TAG, "Framework stopped");
    }
    return ESP_OK;
}

// Deinitialize the framework
void tab5duino_deinit(void) {
    tab5duino_stop();
    deinit_hardware_subsystems();
    
    if (g_tab5duino_instance.config.enable_usb_serial) {
        usb_serial_jtag_driver_uninstall();
    }

    memset(&g_tab5duino_instance, 0, sizeof(tab5duino_instance_t));
    ESP_LOGI(TAG, "Framework deinitialized");
}

// Main loop task
static void loop_task(void* parameter) {
    ESP_LOGI(TAG, "Starting user setup()");
    
    // Call user setup function
    setup();
    g_tab5duino_instance.user_setup_called = true;
    
    ESP_LOGI(TAG, "User setup() complete, starting loop()");

    // Main loop
    while (1) {
        loop();
        taskYIELD();  // Allow other tasks to run
    }
}

// Initialize hardware subsystems
static esp_err_t init_hardware_subsystems(void) {
    esp_err_t ret = ESP_OK;

    // Initialize subsystems based on configuration
    if (g_tab5duino_instance.config.auto_init_display) {
        ret = tab5duino_init_subsystem(TAB5_SUBSYSTEM_DISPLAY);
        if (ret != ESP_OK) return ret;
    }

    if (g_tab5duino_instance.config.auto_init_touch) {
        ret = tab5duino_init_subsystem(TAB5_SUBSYSTEM_TOUCH);
        if (ret != ESP_OK) return ret;
    }

    if (g_tab5duino_instance.config.auto_init_imu) {
        ret = tab5duino_init_subsystem(TAB5_SUBSYSTEM_IMU);
        if (ret != ESP_OK) return ret;
    }

    if (g_tab5duino_instance.config.auto_init_audio) {
        ret = tab5duino_init_subsystem(TAB5_SUBSYSTEM_AUDIO);
        if (ret != ESP_OK) return ret;
    }

    // Always initialize power management
    ret = tab5duino_init_subsystem(TAB5_SUBSYSTEM_POWER);
    if (ret != ESP_OK) return ret;

    // Initialize LVGL after display and touch if enabled
    if (g_tab5duino_instance.config.auto_init_lvgl) {
        ret = tab5duino_init_subsystem(TAB5_SUBSYSTEM_LVGL);
        if (ret != ESP_OK) return ret;
    }

    return ESP_OK;
}

// Deinitialize hardware subsystems
static void deinit_hardware_subsystems(void) {
    for (int i = TAB5_SUBSYSTEM_COUNT - 1; i >= 0; i--) {
        if (g_tab5duino_instance.subsystem_states[i] != TAB5_SUBSYSTEM_UNINITIALIZED) {
            tab5duino_deinit_subsystem((tab5_subsystem_t)i);
        }
    }
}

// Subsystem management functions
esp_err_t tab5duino_init_subsystem(tab5_subsystem_t subsystem) {
    if (subsystem >= TAB5_SUBSYSTEM_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    if (g_tab5duino_instance.subsystem_states[subsystem] != TAB5_SUBSYSTEM_UNINITIALIZED) {
        ESP_LOGW(TAG, "%s subsystem already initialized", subsystem_names[subsystem]);
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing %s subsystem", subsystem_names[subsystem]);
    g_tab5duino_instance.subsystem_states[subsystem] = TAB5_SUBSYSTEM_INITIALIZING;

    esp_err_t ret = ESP_OK;

    switch (subsystem) {
        case TAB5_SUBSYSTEM_DISPLAY:
            // Initialize display driver (placeholder)
            // ret = display_init();
            break;
        
        case TAB5_SUBSYSTEM_TOUCH:
            // Initialize GT911 touch driver (placeholder)
            // ret = touch_init();
            break;
        
        case TAB5_SUBSYSTEM_IMU:
            // Initialize BMI270 IMU (placeholder)
            // ret = imu_init();
            break;
        
        case TAB5_SUBSYSTEM_AUDIO:
            // Initialize PDM audio (placeholder)
            // ret = audio_init();
            break;
        
        case TAB5_SUBSYSTEM_POWER:
            // Initialize power management (placeholder)
            // ret = power_init();
            break;
        
        case TAB5_SUBSYSTEM_USB:
            // Initialize USB OTG (placeholder)
            // ret = usb_init();
            break;
        
        case TAB5_SUBSYSTEM_WIFI:
            // Initialize WiFi (placeholder)
            // ret = wifi_init();
            break;
        
        case TAB5_SUBSYSTEM_LVGL:
            // Initialize LVGL with Tab5duino integration
            {
                static lvgl_tab5_handle_t lvgl_handle = NULL;
                ret = lvgl_tab5_init(&LVGL_TAB5_CONFIG_DEFAULT, &lvgl_handle);
                if (ret == ESP_OK) {
                    ret = lvgl_tab5_start(lvgl_handle);
                    if (ret == ESP_OK && on_lvgl_ready) {
                        on_lvgl_ready();
                    }
                }
            }
            break;
        
        default:
            ret = ESP_ERR_NOT_SUPPORTED;
            break;
    }

    if (ret == ESP_OK) {
        g_tab5duino_instance.subsystem_states[subsystem] = TAB5_SUBSYSTEM_READY;
        ESP_LOGI(TAG, "%s subsystem initialized successfully", subsystem_names[subsystem]);
    } else {
        g_tab5duino_instance.subsystem_states[subsystem] = TAB5_SUBSYSTEM_ERROR;
        ESP_LOGE(TAG, "Failed to initialize %s subsystem: %s", 
                 subsystem_names[subsystem], esp_err_to_name(ret));
        
        // Call error handler if available
        if (on_subsystem_error) {
            on_subsystem_error(subsystem, ret);
        }
    }

    return ret;
}

esp_err_t tab5duino_deinit_subsystem(tab5_subsystem_t subsystem) {
    if (subsystem >= TAB5_SUBSYSTEM_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    if (g_tab5duino_instance.subsystem_states[subsystem] == TAB5_SUBSYSTEM_UNINITIALIZED) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing %s subsystem", subsystem_names[subsystem]);

    // Subsystem-specific deinitialization would go here
    switch (subsystem) {
        case TAB5_SUBSYSTEM_DISPLAY:
            // display_deinit();
            break;
        case TAB5_SUBSYSTEM_TOUCH:
            // touch_deinit();
            break;
        case TAB5_SUBSYSTEM_IMU:
            // imu_deinit();
            break;
        case TAB5_SUBSYSTEM_AUDIO:
            // audio_deinit();
            break;
        case TAB5_SUBSYSTEM_POWER:
            // power_deinit();
            break;
        case TAB5_SUBSYSTEM_USB:
            // usb_deinit();
            break;
        case TAB5_SUBSYSTEM_WIFI:
            // wifi_deinit();
            break;
        default:
            break;
    }

    g_tab5duino_instance.subsystem_states[subsystem] = TAB5_SUBSYSTEM_UNINITIALIZED;
    return ESP_OK;
}

tab5_subsystem_state_t tab5duino_get_subsystem_state(tab5_subsystem_t subsystem) {
    if (subsystem >= TAB5_SUBSYSTEM_COUNT) {
        return TAB5_SUBSYSTEM_ERROR;
    }
    return g_tab5duino_instance.subsystem_states[subsystem];
}

// Framework information functions
const char* tab5duino_get_version_string(void) {
    static char version_str[32];
    snprintf(version_str, sizeof(version_str), "%d.%d.%d", 
             TAB5DUINO_VERSION_MAJOR, TAB5DUINO_VERSION_MINOR, TAB5DUINO_VERSION_PATCH);
    return version_str;
}

uint64_t tab5duino_get_boot_time_us(void) {
    return g_tab5duino_instance.boot_time_us;
}

bool tab5duino_is_ready(void) {
    return g_tab5duino_instance.framework_initialized && 
           g_tab5duino_instance.user_setup_called &&
           g_tab5duino_instance.loop_task_handle != NULL;
}

// Memory management helpers
void* tab5duino_malloc(size_t size) {
    if (g_tab5duino_instance.config.enable_psram) {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    return malloc(size);
}

void* tab5duino_calloc(size_t num, size_t size) {
    if (g_tab5duino_instance.config.enable_psram) {
        return heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    return calloc(num, size);
}

void* tab5duino_realloc(void* ptr, size_t size) {
    if (g_tab5duino_instance.config.enable_psram) {
        return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    return realloc(ptr, size);
}

void tab5duino_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

#ifdef __cplusplus

// C++ Framework Class Implementation
Tab5duinoFramework* Tab5duinoFramework::instance = nullptr;

Tab5duinoFramework::Tab5duinoFramework() : initialized(false) {
}

Tab5duinoFramework::~Tab5duinoFramework() {
    if (initialized) {
        end();
    }
}

Tab5duinoFramework& Tab5duinoFramework::getInstance() {
    if (instance == nullptr) {
        instance = new Tab5duinoFramework();
    }
    return *instance;
}

esp_err_t Tab5duinoFramework::begin(const tab5duino_config_t* config) {
    esp_err_t ret = tab5duino_init(config);
    if (ret == ESP_OK) {
        initialized = true;
    }
    return ret;
}

esp_err_t Tab5duinoFramework::start() {
    return tab5duino_start();
}

void Tab5duinoFramework::end() {
    if (initialized) {
        tab5duino_deinit();
        initialized = false;
    }
}

esp_err_t Tab5duinoFramework::initSubsystem(tab5_subsystem_t subsystem) {
    return tab5duino_init_subsystem(subsystem);
}

esp_err_t Tab5duinoFramework::deinitSubsystem(tab5_subsystem_t subsystem) {
    return tab5duino_deinit_subsystem(subsystem);
}

tab5_subsystem_state_t Tab5duinoFramework::getSubsystemState(tab5_subsystem_t subsystem) {
    return tab5duino_get_subsystem_state(subsystem);
}

const char* Tab5duinoFramework::getVersionString() {
    return tab5duino_get_version_string();
}

uint64_t Tab5duinoFramework::getBootTimeUs() {
    return tab5duino_get_boot_time_us();
}

bool Tab5duinoFramework::isReady() {
    return tab5duino_is_ready();
}

void* Tab5duinoFramework::malloc(size_t size) {
    return tab5duino_malloc(size);
}

void* Tab5duinoFramework::calloc(size_t num, size_t size) {
    return tab5duino_calloc(num, size);
}

void* Tab5duinoFramework::realloc(void* ptr, size_t size) {
    return tab5duino_realloc(ptr, size);
}

void Tab5duinoFramework::free(void* ptr) {
    tab5duino_free(ptr);
}

// Global framework instance
Tab5duinoFramework& Tab5duino = Tab5duinoFramework::getInstance();

#endif // __cplusplus