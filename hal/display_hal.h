/*
 * Display HAL for M5Stack Tab5
 * 1280x720 MIPI-DSI Display with PPA Hardware Acceleration
 */

#ifndef _DISPLAY_HAL_H_
#define _DISPLAY_HAL_H_

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

// Display specifications
#define DISPLAY_WIDTH           1280
#define DISPLAY_HEIGHT          720
#define DISPLAY_BITS_PER_PIXEL  16    // RGB565
#define DISPLAY_BUFFER_SIZE     (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2)
#define DISPLAY_REFRESH_RATE    60    // Hz
#define DISPLAY_PIXEL_CLOCK_MHZ 74    // MHz for 1280x720@60Hz

// Display configuration structure
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    uint32_t pixel_clock_hz;
    bool enable_ppa_acceleration;
    bool enable_double_buffer;
    bool enable_vsync;
    uint8_t backlight_level;  // 0-255
} display_config_t;

// Display handle
typedef struct display_handle_s* display_handle_t;

// Color format definitions
typedef enum {
    DISPLAY_COLOR_RGB565,
    DISPLAY_COLOR_RGB888,
    DISPLAY_COLOR_ARGB8888
} display_color_format_t;

// Display rotation
typedef enum {
    DISPLAY_ROTATION_0 = 0,
    DISPLAY_ROTATION_90,
    DISPLAY_ROTATION_180,
    DISPLAY_ROTATION_270
} display_rotation_t;

// Frame buffer information
typedef struct {
    void* buffer;
    size_t size;
    uint16_t width;
    uint16_t height;
    display_color_format_t format;
    bool is_ready;
} display_framebuffer_t;

// Display initialization and management
esp_err_t display_hal_init(const display_config_t* config, display_handle_t* handle);
esp_err_t display_hal_deinit(display_handle_t handle);
esp_err_t display_hal_start(display_handle_t handle);
esp_err_t display_hal_stop(display_handle_t handle);

// Display control
esp_err_t display_hal_set_backlight(display_handle_t handle, uint8_t level);
uint8_t display_hal_get_backlight(display_handle_t handle);
esp_err_t display_hal_set_rotation(display_handle_t handle, display_rotation_t rotation);
display_rotation_t display_hal_get_rotation(display_handle_t handle);

// Frame buffer management
esp_err_t display_hal_get_framebuffer(display_handle_t handle, display_framebuffer_t* fb);
esp_err_t display_hal_swap_buffers(display_handle_t handle);
esp_err_t display_hal_wait_vsync(display_handle_t handle, uint32_t timeout_ms);

// Drawing operations
esp_err_t display_hal_clear(display_handle_t handle, uint16_t color);
esp_err_t display_hal_fill_rect(display_handle_t handle, 
                               uint16_t x, uint16_t y, 
                               uint16_t width, uint16_t height, 
                               uint16_t color);
esp_err_t display_hal_draw_pixel(display_handle_t handle, uint16_t x, uint16_t y, uint16_t color);
esp_err_t display_hal_draw_bitmap(display_handle_t handle,
                                 uint16_t x, uint16_t y,
                                 uint16_t width, uint16_t height,
                                 const uint16_t* bitmap);

// PPA acceleration functions (if enabled)
esp_err_t display_hal_ppa_fill(display_handle_t handle,
                              uint16_t x, uint16_t y,
                              uint16_t width, uint16_t height,
                              uint16_t color);
esp_err_t display_hal_ppa_blend(display_handle_t handle,
                               uint16_t dst_x, uint16_t dst_y,
                               const uint16_t* src_buffer,
                               uint16_t src_x, uint16_t src_y,
                               uint16_t width, uint16_t height,
                               uint8_t alpha);

// Information functions
esp_err_t display_hal_get_info(display_handle_t handle, display_config_t* info);
bool display_hal_is_ready(display_handle_t handle);

// Callback types
typedef void (*display_vsync_callback_t)(display_handle_t handle, void* user_data);
typedef void (*display_draw_complete_callback_t)(display_handle_t handle, void* user_data);

// Callback registration
esp_err_t display_hal_register_vsync_callback(display_handle_t handle, 
                                            display_vsync_callback_t callback,
                                            void* user_data);
esp_err_t display_hal_register_draw_complete_callback(display_handle_t handle,
                                                    display_draw_complete_callback_t callback,
                                                    void* user_data);

// Color conversion utilities
static inline uint16_t display_rgb_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static inline void display_rgb565_to_rgb(uint16_t color, uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = (color >> 8) & 0xF8;
    *g = (color >> 3) & 0xFC;
    *b = (color << 3) & 0xF8;
}

// Common color definitions
#define DISPLAY_COLOR_BLACK     0x0000
#define DISPLAY_COLOR_WHITE     0xFFFF
#define DISPLAY_COLOR_RED       0xF800
#define DISPLAY_COLOR_GREEN     0x07E0
#define DISPLAY_COLOR_BLUE      0x001F
#define DISPLAY_COLOR_CYAN      0x07FF
#define DISPLAY_COLOR_MAGENTA   0xF81F
#define DISPLAY_COLOR_YELLOW    0xFFE0
#define DISPLAY_COLOR_ORANGE    0xFD20
#define DISPLAY_COLOR_PURPLE    0x8010
#define DISPLAY_COLOR_GRAY      0x8410
#define DISPLAY_COLOR_DARK_GRAY 0x4208
#define DISPLAY_COLOR_LIGHT_GRAY 0xC618

// Default configuration
extern const display_config_t DISPLAY_CONFIG_DEFAULT;

#ifdef __cplusplus
}
#endif

#endif // _DISPLAY_HAL_H_