/**
 * @file lvgl_tab5duino.cpp
 * @brief LVGL Integration Implementation for Tab5duino Framework
 */

#include "lvgl_tab5duino.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <cstring>

static const char* TAG = "LVGL_Tab5";

// Default configuration
const lvgl_tab5_config_t LVGL_TAB5_CONFIG_DEFAULT = {
    .enable_ppa_acceleration = LVGL_TAB5_USE_PPA_ACCELERATION,
    .enable_vsync = LVGL_TAB5_USE_VSYNC,
    .enable_double_buffer = true,
    .buffer_lines = LVGL_TAB5_BUFFER_SIZE_LINES,
    .rotation = 0,
    .enable_multi_touch = true,
    .enable_gestures = true,
    .touch_threshold = LVGL_TAB5_TOUCH_THRESHOLD,
    .gesture_threshold = LVGL_TAB5_GESTURE_THRESHOLD,
    .task_priority = LVGL_TAB5_TASK_PRIORITY,
    .task_stack_size = LVGL_TAB5_TASK_STACK_SIZE,
    .task_core = LVGL_TAB5_TASK_CORE,
    .tick_period_ms = LVGL_TAB5_TICK_PERIOD_MS,
    .use_psram_buffers = LVGL_TAB5_USE_PSRAM_BUFFERS,
    .psram_cache_size = LVGL_TAB5_PSRAM_CACHE_SIZE
};

// Internal handle structure
struct lvgl_tab5_handle_s {
    lvgl_tab5_context_t context;
    lvgl_tab5_config_t config;
    
    // Callbacks
    lvgl_ready_callback_t ready_callback;
    void* ready_user_data;
    lvgl_error_callback_t error_callback;
    void* error_user_data;
    
    // Performance tracking
    uint64_t frame_count;
    uint64_t last_perf_time;
    uint16_t fps_average;
    uint8_t cpu_usage;
    
    // State
    bool started;
};

// Forward declarations
static void lvgl_task(void* parameter);
static void display_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
static void display_wait_cb(lv_disp_drv_t* disp_drv);
static void touchpad_read_cb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data);
static void log_print_cb(const char* buf);
static uint32_t tick_get_cb(void);

// LVGL Timer callback
static esp_timer_handle_t lvgl_tick_timer = NULL;
static void lvgl_tick_task(void* arg) {
    lv_tick_inc(LVGL_TAB5_TICK_PERIOD_MS);
}

// Memory allocation functions for LVGL
void* tab5duino_lvgl_malloc(size_t size) {
    void* ptr = nullptr;
    
    // Try PSRAM first if available and enabled
    if (esp_psram_is_initialized()) {
        ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (ptr) {
            return ptr;
        }
    }
    
    // Fallback to internal RAM
    ptr = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!ptr) {
        ESP_LOGE(TAG, "Failed to allocate %zu bytes for LVGL", size);
    }
    
    return ptr;
}

void* tab5duino_lvgl_realloc(void* ptr, size_t size) {
    // For realloc, we need to check where the original memory was allocated
    if (!ptr) {
        return tab5duino_lvgl_malloc(size);
    }
    
    if (size == 0) {
        tab5duino_lvgl_free(ptr);
        return nullptr;
    }
    
    void* new_ptr = tab5duino_lvgl_malloc(size);
    if (new_ptr && ptr) {
        // Copy old data (we don't know the original size, so copy conservatively)
        size_t copy_size = size; // Assume we're expanding
        memcpy(new_ptr, ptr, copy_size);
        tab5duino_lvgl_free(ptr);
    }
    
    return new_ptr;
}

void tab5duino_lvgl_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

esp_err_t lvgl_tab5_init(const lvgl_tab5_config_t* config, lvgl_tab5_handle_t* handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing LVGL Tab5duino integration");
    
    // Allocate handle
    *handle = (lvgl_tab5_handle_t)calloc(1, sizeof(struct lvgl_tab5_handle_s));
    if (!*handle) {
        ESP_LOGE(TAG, "Failed to allocate handle");
        return ESP_ERR_NO_MEM;
    }
    
    lvgl_tab5_handle_t h = *handle;
    
    // Copy configuration
    if (config) {
        memcpy(&h->config, config, sizeof(lvgl_tab5_config_t));
    } else {
        memcpy(&h->config, &LVGL_TAB5_CONFIG_DEFAULT, sizeof(lvgl_tab5_config_t));
    }
    
    esp_err_t ret = ESP_OK;
    
    // Initialize LVGL
    lv_init();
    
    // Set up LVGL logging
    lv_log_register_print_cb(log_print_cb);
    
    // Create mutex for thread safety
    h->context.lvgl_mutex = xSemaphoreCreateMutex();
    if (!h->context.lvgl_mutex) {
        ESP_LOGE(TAG, "Failed to create LVGL mutex");
        ret = ESP_ERR_NO_MEM;
        goto cleanup;
    }
    
    // Initialize display HAL
    display_config_t display_config = {
        .width = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT,
        .bits_per_pixel = DISPLAY_BITS_PER_PIXEL,
        .pixel_clock_hz = DISPLAY_PIXEL_CLOCK_MHZ * 1000000,
        .enable_ppa_acceleration = h->config.enable_ppa_acceleration,
        .enable_double_buffer = h->config.enable_double_buffer,
        .enable_vsync = h->config.enable_vsync,
        .backlight_level = 255
    };
    
    ret = display_hal_init(&display_config, &h->context.display_hal);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display HAL: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    
    // Initialize touch HAL
    touch_config_t touch_config = {
        .i2c_address = 0x5D,  // GT911 default address
        .interrupt_pin = 21,   // Configure based on actual hardware
        .reset_pin = 38,
        .sda_pin = 6,
        .scl_pin = 7,
        .i2c_frequency = 400000,
        .enable_multi_touch = h->config.enable_multi_touch,
        .enable_gestures = h->config.enable_gestures,
        .debounce_ms = 10,
        .sensitivity = 128,
        .flip_x = false,
        .flip_y = false,
        .swap_xy = false
    };
    
    ret = touch_hal_init(&touch_config, &h->context.touch_hal);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize touch HAL: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    
    // Allocate display buffers
    size_t buffer_size = DISPLAY_WIDTH * h->config.buffer_lines * sizeof(lv_color_t);
    
    if (h->config.use_psram_buffers && esp_psram_is_initialized()) {
        h->context.buffer1 = (lv_color_t*)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (h->config.enable_double_buffer) {
            h->context.buffer2 = (lv_color_t*)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        }
        ESP_LOGI(TAG, "Allocated display buffers in PSRAM (%zu bytes each)", buffer_size);
    } else {
        h->context.buffer1 = (lv_color_t*)heap_caps_malloc(buffer_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
        if (h->config.enable_double_buffer) {
            h->context.buffer2 = (lv_color_t*)heap_caps_malloc(buffer_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
        }
        ESP_LOGI(TAG, "Allocated display buffers in internal RAM (%zu bytes each)", buffer_size);
    }
    
    if (!h->context.buffer1 || (h->config.enable_double_buffer && !h->context.buffer2)) {
        ESP_LOGE(TAG, "Failed to allocate display buffers");
        ret = ESP_ERR_NO_MEM;
        goto cleanup;
    }
    
    // Initialize LVGL display buffer
    h->context.draw_buffer = (lv_disp_draw_buf_t*)malloc(sizeof(lv_disp_draw_buf_t));
    if (!h->context.draw_buffer) {
        ESP_LOGE(TAG, "Failed to allocate draw buffer structure");
        ret = ESP_ERR_NO_MEM;
        goto cleanup;
    }
    
    lv_disp_draw_buf_init(h->context.draw_buffer, h->context.buffer1, h->context.buffer2,
                          DISPLAY_WIDTH * h->config.buffer_lines);
    
    // Initialize LVGL display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISPLAY_WIDTH;
    disp_drv.ver_res = DISPLAY_HEIGHT;
    disp_drv.flush_cb = display_flush_cb;
    disp_drv.wait_cb = display_wait_cb;
    disp_drv.draw_buf = h->context.draw_buffer;
    disp_drv.user_data = h;
    
    // Register display
    h->context.display = lv_disp_drv_register(&disp_drv);
    if (!h->context.display) {
        ESP_LOGE(TAG, "Failed to register LVGL display driver");
        ret = ESP_FAIL;
        goto cleanup;
    }
    
    // Initialize LVGL input device driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read_cb;
    indev_drv.user_data = h;
    
    // Register input device
    h->context.touchpad = lv_indev_drv_register(&indev_drv);
    if (!h->context.touchpad) {
        ESP_LOGE(TAG, "Failed to register LVGL input device driver");
        ret = ESP_FAIL;
        goto cleanup;
    }
    
    // Initialize tick timer
    esp_timer_create_args_t timer_args = {
        .callback = lvgl_tick_task,
        .arg = h,
        .name = "lvgl_tick"
    };
    
    ret = esp_timer_create(&timer_args, &lvgl_tick_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LVGL tick timer: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    
    h->context.initialized = true;
    ESP_LOGI(TAG, "LVGL Tab5duino integration initialized successfully");
    
    return ESP_OK;

cleanup:
    if (h) {
        lvgl_tab5_deinit(h);
        *handle = NULL;
    }
    return ret;
}

esp_err_t lvgl_tab5_start(lvgl_tab5_handle_t handle) {
    if (!handle || !handle->context.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (handle->started) {
        ESP_LOGW(TAG, "LVGL already started");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Starting LVGL task");
    
    // Start display HAL
    esp_err_t ret = display_hal_start(handle->context.display_hal);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start display HAL: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Start touch HAL
    ret = touch_hal_start(handle->context.touch_hal);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start touch HAL: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Start tick timer
    ret = esp_timer_start_periodic(lvgl_tick_timer, handle->config.tick_period_ms * 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start tick timer: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create LVGL task
    BaseType_t result = xTaskCreatePinnedToCore(
        lvgl_task,
        "lvgl_task",
        handle->config.task_stack_size,
        handle,
        handle->config.task_priority,
        &handle->context.lvgl_task,
        handle->config.task_core
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LVGL task");
        esp_timer_stop(lvgl_tick_timer);
        return ESP_ERR_NO_MEM;
    }
    
    handle->started = true;
    handle->last_perf_time = esp_timer_get_time();
    
    // Call ready callback if registered
    if (handle->ready_callback) {
        handle->ready_callback(handle, handle->ready_user_data);
    }
    
    ESP_LOGI(TAG, "LVGL started successfully");
    return ESP_OK;
}

esp_err_t lvgl_tab5_stop(lvgl_tab5_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!handle->started) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Stopping LVGL");
    
    // Delete LVGL task
    if (handle->context.lvgl_task) {
        vTaskDelete(handle->context.lvgl_task);
        handle->context.lvgl_task = NULL;
    }
    
    // Stop tick timer
    if (lvgl_tick_timer) {
        esp_timer_stop(lvgl_tick_timer);
    }
    
    // Stop HAL components
    display_hal_stop(handle->context.display_hal);
    touch_hal_stop(handle->context.touch_hal);
    
    handle->started = false;
    ESP_LOGI(TAG, "LVGL stopped");
    
    return ESP_OK;
}

void lvgl_tab5_deinit(lvgl_tab5_handle_t handle) {
    if (!handle) {
        return;
    }
    
    ESP_LOGI(TAG, "Deinitializing LVGL");
    
    // Stop if running
    lvgl_tab5_stop(handle);
    
    // Delete tick timer
    if (lvgl_tick_timer) {
        esp_timer_delete(lvgl_tick_timer);
        lvgl_tick_timer = NULL;
    }
    
    // Deinitialize HAL components
    if (handle->context.display_hal) {
        display_hal_deinit(handle->context.display_hal);
    }
    if (handle->context.touch_hal) {
        touch_hal_deinit(handle->context.touch_hal);
    }
    
    // Free display buffers
    if (handle->context.buffer1) {
        free(handle->context.buffer1);
    }
    if (handle->context.buffer2) {
        free(handle->context.buffer2);
    }
    if (handle->context.draw_buffer) {
        free(handle->context.draw_buffer);
    }
    
    // Delete mutex
    if (handle->context.lvgl_mutex) {
        vSemaphoreDelete(handle->context.lvgl_mutex);
    }
    
    // Free handle
    free(handle);
    
    ESP_LOGI(TAG, "LVGL deinitialized");
}

bool lvgl_tab5_lock(lvgl_tab5_handle_t handle, uint32_t timeout_ms) {
    if (!handle || !handle->context.lvgl_mutex) {
        return false;
    }
    
    TickType_t timeout_ticks = timeout_ms == portMAX_DELAY ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(handle->context.lvgl_mutex, timeout_ticks) == pdTRUE;
}

void lvgl_tab5_unlock(lvgl_tab5_handle_t handle) {
    if (handle && handle->context.lvgl_mutex) {
        xSemaphoreGive(handle->context.lvgl_mutex);
    }
}

// LVGL task implementation
static void lvgl_task(void* parameter) {
    lvgl_tab5_handle_t handle = (lvgl_tab5_handle_t)parameter;
    
    ESP_LOGI(TAG, "LVGL task started");
    
    while (1) {
        // Lock LVGL
        if (xSemaphoreTake(handle->context.lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            // Process LVGL tasks
            uint32_t task_delay = lv_timer_handler();
            
            // Update performance counters
            handle->frame_count++;
            uint64_t now = esp_timer_get_time();
            if (now - handle->last_perf_time >= 1000000) { // 1 second
                handle->fps_average = handle->frame_count;
                handle->frame_count = 0;
                handle->last_perf_time = now;
            }
            
            // Release lock
            xSemaphoreGive(handle->context.lvgl_mutex);
            
            // Delay based on LVGL recommendation
            vTaskDelay(pdMS_TO_TICKS(task_delay));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10)); // Fallback delay
        }
    }
}

// Display flush callback
static void display_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
    lvgl_tab5_handle_t handle = (lvgl_tab5_handle_t)disp_drv->user_data;
    
    if (!handle || !handle->context.display_hal) {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    
    // Calculate buffer parameters
    uint16_t width = area->x2 - area->x1 + 1;
    uint16_t height = area->y2 - area->y1 + 1;
    
    // Use PPA acceleration if enabled
    if (handle->config.enable_ppa_acceleration) {
        esp_err_t ret = display_hal_ppa_blend(handle->context.display_hal,
                                            area->x1, area->y1,
                                            (const uint16_t*)color_p,
                                            0, 0, width, height, 255);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "PPA blend failed, using software fallback");
            display_hal_draw_bitmap(handle->context.display_hal,
                                  area->x1, area->y1, width, height,
                                  (const uint16_t*)color_p);
        }
    } else {
        display_hal_draw_bitmap(handle->context.display_hal,
                              area->x1, area->y1, width, height,
                              (const uint16_t*)color_p);
    }
    
    lv_disp_flush_ready(disp_drv);
}

// Display wait callback for VSYNC
static void display_wait_cb(lv_disp_drv_t* disp_drv) {
    lvgl_tab5_handle_t handle = (lvgl_tab5_handle_t)disp_drv->user_data;
    
    if (handle && handle->context.display_hal && handle->config.enable_vsync) {
        display_hal_wait_vsync(handle->context.display_hal, 20); // 20ms timeout
    }
}

// Touch input callback
static void touchpad_read_cb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
    lvgl_tab5_handle_t handle = (lvgl_tab5_handle_t)indev_drv->user_data;
    
    if (!handle || !handle->context.touch_hal) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    
    touch_point_t points[TOUCH_MAX_POINTS];
    uint8_t point_count = 0;
    
    esp_err_t ret = touch_hal_read_points(handle->context.touch_hal, points, &point_count);
    if (ret != ESP_OK || point_count == 0) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    
    // Use first valid touch point
    for (uint8_t i = 0; i < point_count; i++) {
        if (touch_point_is_valid(&points[i])) {
            data->state = LV_INDEV_STATE_PR;
            data->point.x = points[i].x;
            data->point.y = points[i].y;
            return;
        }
    }
    
    data->state = LV_INDEV_STATE_REL;
}

// LVGL log callback
static void log_print_cb(const char* buf) {
    ESP_LOGI("LVGL", "%s", buf);
}

// Additional API implementations
esp_err_t lvgl_tab5_refresh_display(lvgl_tab5_handle_t handle) {
    if (!handle || !handle->context.display) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (lvgl_tab5_lock(handle, 1000)) {
        lv_obj_invalidate(lv_scr_act());
        lvgl_tab5_unlock(handle);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

lv_disp_t* lvgl_tab5_get_display(lvgl_tab5_handle_t handle) {
    return handle ? handle->context.display : NULL;
}

lv_indev_t* lvgl_tab5_get_touchpad(lvgl_tab5_handle_t handle) {
    return handle ? handle->context.touchpad : NULL;
}

esp_err_t lvgl_tab5_get_performance_stats(lvgl_tab5_handle_t handle,
                                         uint16_t* fps_avg,
                                         uint8_t* cpu_usage,
                                         uint32_t* mem_used) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (fps_avg) *fps_avg = handle->fps_average;
    if (cpu_usage) *cpu_usage = handle->cpu_usage;
    if (mem_used) {
        multi_heap_info_t info;
        heap_caps_get_info(&info, MALLOC_CAP_8BIT);
        *mem_used = info.total_allocated_bytes;
    }
    
    return ESP_OK;
}

esp_err_t lvgl_tab5_set_brightness(lvgl_tab5_handle_t handle, uint8_t brightness) {
    if (!handle || !handle->context.display_hal) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return display_hal_set_backlight(handle->context.display_hal, brightness);
}

uint8_t lvgl_tab5_get_brightness(lvgl_tab5_handle_t handle) {
    if (!handle || !handle->context.display_hal) {
        return 0;
    }
    
    return display_hal_get_backlight(handle->context.display_hal);
}

bool lvgl_tab5_is_ready(lvgl_tab5_handle_t handle) {
    return handle && handle->context.initialized && handle->started;
}

lv_obj_t* lvgl_tab5_create_screen(lvgl_tab5_handle_t handle) {
    if (!handle || !lvgl_tab5_lock(handle, 1000)) {
        return NULL;
    }
    
    lv_obj_t* screen = lv_obj_create(NULL);
    lvgl_tab5_unlock(handle);
    
    return screen;
}

esp_err_t lvgl_tab5_load_screen(lvgl_tab5_handle_t handle, lv_obj_t* screen) {
    if (!handle || !screen) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (lvgl_tab5_lock(handle, 1000)) {
        lv_scr_load(screen);
        lvgl_tab5_unlock(handle);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

// C++ wrapper implementation
#ifdef __cplusplus

LVGLTab5duino::LVGLTab5duino() : handle_(nullptr), initialized_(false) {
}

LVGLTab5duino::~LVGLTab5duino() {
    if (initialized_) {
        end();
    }
}

esp_err_t LVGLTab5duino::begin(const lvgl_tab5_config_t* config) {
    if (initialized_) {
        return ESP_OK;
    }
    
    esp_err_t ret = lvgl_tab5_init(config, &handle_);
    if (ret == ESP_OK) {
        initialized_ = true;
    }
    
    return ret;
}

esp_err_t LVGLTab5duino::start() {
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return lvgl_tab5_start(handle_);
}

esp_err_t LVGLTab5duino::stop() {
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return lvgl_tab5_stop(handle_);
}

void LVGLTab5duino::end() {
    if (initialized_) {
        lvgl_tab5_deinit(handle_);
        handle_ = nullptr;
        initialized_ = false;
    }
}

bool LVGLTab5duino::lock(uint32_t timeout_ms) {
    return initialized_ ? lvgl_tab5_lock(handle_, timeout_ms) : false;
}

void LVGLTab5duino::unlock() {
    if (initialized_) {
        lvgl_tab5_unlock(handle_);
    }
}

lv_obj_t* LVGLTab5duino::createScreen() {
    return initialized_ ? lvgl_tab5_create_screen(handle_) : nullptr;
}

esp_err_t LVGLTab5duino::loadScreen(lv_obj_t* screen) {
    return initialized_ ? lvgl_tab5_load_screen(handle_, screen) : ESP_ERR_INVALID_STATE;
}

bool LVGLTab5duino::isReady() {
    return initialized_ ? lvgl_tab5_is_ready(handle_) : false;
}

lv_disp_t* LVGLTab5duino::getDisplay() {
    return initialized_ ? lvgl_tab5_get_display(handle_) : nullptr;
}

lv_indev_t* LVGLTab5duino::getTouchpad() {
    return initialized_ ? lvgl_tab5_get_touchpad(handle_) : nullptr;
}

// Global instance
static LVGLTab5duino g_lvgl_instance;
LVGLTab5duino& LVGL = g_lvgl_instance;

#endif // __cplusplus