/**
 * @file main.cpp
 * @brief LVGL Performance Demonstration for M5Stack Tab5
 * 
 * This example showcases the full potential of the 1280x720 display with
 * PPA hardware acceleration, demonstrating smooth animations, complex layouts,
 * and high frame rate performance.
 */

#include <Arduino.h>
#include <Tab5duino.h>
#include <lvgl_tab5duino.h>
#include <vector>

// Performance test objects
static lv_obj_t* main_screen;
static lv_obj_t* chart;
static lv_obj_t* meter;
static lv_obj_t* spinner;
static lv_obj_t* arc_progress;
static lv_chart_series_t* chart_series;
static std::vector<lv_obj_t*> animated_objects;

// Performance tracking
static uint32_t frame_count = 0;
static uint32_t last_fps_time = 0;
static float current_fps = 0.0f;

// Global LVGL instance
extern LVGLTab5duino& LVGL;

// Animation callback for floating objects
static void float_animation_cb(void* obj, int32_t value) {
    lv_obj_t* target = (lv_obj_t*)obj;
    lv_coord_t y = LV_MAX(50, value);
    lv_obj_set_y(target, y);
}

// Create animated floating objects
void create_floating_objects() {
    const int num_objects = 20;
    const lv_color_t colors[] = {
        lv_color_hex(0xFF6B6B), lv_color_hex(0x4ECDC4), lv_color_hex(0x45B7D1),
        lv_color_hex(0xFFA07A), lv_color_hex(0x98D8C8), lv_color_hex(0xA8E6CF)
    };
    
    for (int i = 0; i < num_objects; i++) {
        lv_obj_t* obj = lv_obj_create(main_screen);
        lv_obj_set_size(obj, 30, 30);
        
        // Random position
        lv_coord_t x = random(50, 1230);
        lv_coord_t y = random(100, 600);
        lv_obj_set_pos(obj, x, y);
        
        // Style
        lv_obj_set_style_bg_color(obj, colors[i % 6], LV_PART_MAIN);
        lv_obj_set_style_radius(obj, 15, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(obj, 10, LV_PART_MAIN);
        lv_obj_set_style_shadow_opa(obj, LV_OPA_30, LV_PART_MAIN);
        
        // Create floating animation
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, obj);
        lv_anim_set_exec_cb(&anim, float_animation_cb);
        lv_anim_set_values(&anim, y, y + random(-100, 100));
        lv_anim_set_time(&anim, random(2000, 5000));
        lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_playback_time(&anim, random(1000, 3000));
        lv_anim_start(&anim);
        
        animated_objects.push_back(obj);
    }
}

// Update performance chart
static void update_performance_chart() {
    // Calculate FPS
    uint32_t now = millis();
    frame_count++;
    
    if (now - last_fps_time >= 1000) {
        current_fps = (float)frame_count * 1000.0f / (now - last_fps_time);
        frame_count = 0;
        last_fps_time = now;
        
        // Add point to chart
        lv_chart_set_next_value(chart, chart_series, (lv_coord_t)current_fps);
        
        ESP_LOGI("Performance", "FPS: %.1f", current_fps);
    }
}

// Update meter with CPU usage simulation
static void update_meter() {
    static uint32_t last_update = 0;
    uint32_t now = millis();
    
    if (now - last_update >= 100) {
        last_update = now;
        
        // Simulate CPU usage based on FPS
        int32_t cpu_usage = LV_MAX(10, LV_MIN(100, 100 - (int32_t)current_fps));
        lv_meter_set_indicator_value(meter, 
                                   (lv_meter_indicator_t*)lv_obj_get_user_data(meter),
                                   cpu_usage);
    }
}

// Progress arc animation
static void update_progress_arc() {
    static uint32_t last_update = 0;
    static int32_t progress = 0;
    static int32_t direction = 1;
    
    uint32_t now = millis();
    if (now - last_update >= 50) {
        last_update = now;
        
        progress += direction * 2;
        if (progress >= 100) {
            progress = 100;
            direction = -1;
        } else if (progress <= 0) {
            progress = 0;
            direction = 1;
        }
        
        lv_arc_set_value(arc_progress, progress);
    }
}

void create_performance_ui() {
    // Create main screen with gradient background
    main_screen = LVGL.createScreen();
    
    // Create gradient background
    static lv_style_t bg_style;
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, lv_color_hex(0x001122));
    lv_style_set_bg_grad_color(&bg_style, lv_color_hex(0x003366));
    lv_style_set_bg_grad_dir(&bg_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(main_screen, &bg_style, LV_PART_MAIN);
    
    // Title
    lv_obj_t* title = lv_label_create(main_screen);
    lv_label_set_text(title, "M5Stack Tab5 - LVGL Performance Demo");
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // FPS Chart
    chart = lv_chart_create(main_screen);
    lv_obj_set_size(chart, 300, 150);
    lv_obj_align(chart, LV_ALIGN_TOP_LEFT, 20, 60);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, 50);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 70);
    
    chart_series = lv_chart_add_series(chart, lv_color_hex(0x00FF88), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_all_value(chart, chart_series, 60);
    
    // Chart title
    lv_obj_t* chart_title = lv_label_create(main_screen);
    lv_label_set_text(chart_title, "Frame Rate (FPS)");
    lv_obj_set_style_text_color(chart_title, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(chart_title, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    // CPU Usage Meter
    meter = lv_meter_create(main_screen);
    lv_obj_set_size(meter, 180, 180);
    lv_obj_align(meter, LV_ALIGN_TOP_RIGHT, -20, 60);
    
    // Add meter scale
    lv_meter_scale_t* scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 21, 2, 8, lv_color_white());
    lv_meter_set_scale_major_ticks(meter, scale, 5, 4, 15, lv_color_white(), 10);
    lv_meter_set_scale_range(meter, scale, 0, 100, 240, 120);
    
    // Add needle indicator
    lv_meter_indicator_t* needle = lv_meter_add_needle_line(meter, scale, 4, 
                                                           lv_color_hex(0xFF6600), -10);
    lv_obj_set_user_data(meter, needle);
    
    // Meter title
    lv_obj_t* meter_title = lv_label_create(main_screen);
    lv_label_set_text(meter_title, "CPU Usage %");
    lv_obj_set_style_text_color(meter_title, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(meter_title, meter, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    // Progress Arc
    arc_progress = lv_arc_create(main_screen);
    lv_obj_set_size(arc_progress, 150, 150);
    lv_obj_align(arc_progress, LV_ALIGN_CENTER, 0, -50);
    lv_arc_set_rotation(arc_progress, 270);
    lv_arc_set_bg_angles(arc_progress, 0, 360);
    lv_arc_set_range(arc_progress, 0, 100);
    lv_obj_set_style_arc_width(arc_progress, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_progress, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_progress, lv_color_hex(0x00AAFF), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_progress, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_progress, LV_OBJ_FLAG_CLICKABLE);
    
    // Arc title
    lv_obj_t* arc_title = lv_label_create(main_screen);
    lv_label_set_text(arc_title, "GPU Load");
    lv_obj_set_style_text_color(arc_title, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(arc_title, arc_progress, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    
    // Spinner for continuous animation
    spinner = lv_spinner_create(main_screen, 1000, 60);
    lv_obj_set_size(spinner, 100, 100);
    lv_obj_align(spinner, LV_ALIGN_BOTTOM_LEFT, 50, -50);
    
    // System info labels
    lv_obj_t* info1 = lv_label_create(main_screen);
    lv_label_set_text(info1, "Resolution: 1280×720");
    lv_obj_set_style_text_color(info1, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(info1, LV_ALIGN_BOTTOM_RIGHT, -20, -80);
    
    lv_obj_t* info2 = lv_label_create(main_screen);
    lv_label_set_text(info2, "PSRAM: 32MB");
    lv_obj_set_style_text_color(info2, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(info2, LV_ALIGN_BOTTOM_RIGHT, -20, -60);
    
    lv_obj_t* info3 = lv_label_create(main_screen);
    lv_label_set_text(info3, "PPA: Enabled");
    lv_obj_set_style_text_color(info3, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(info3, LV_ALIGN_BOTTOM_RIGHT, -20, -40);
    
    lv_obj_t* info4 = lv_label_create(main_screen);
    lv_label_set_text(info4, "Target: 60 FPS");
    lv_obj_set_style_text_color(info4, lv_color_hex(0x00FF88), LV_PART_MAIN);
    lv_obj_align(info4, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    
    // Create floating animated objects
    create_floating_objects();
    
    // Load the screen
    LVGL.loadScreen(main_screen);
}

// Performance update timer callback
static void performance_timer_cb(lv_timer_t* timer) {
    update_performance_chart();
    update_meter();
    update_progress_arc();
}

void on_lvgl_ready() {
    ESP_LOGI("LVGL_Performance", "LVGL ready! Creating performance demo...");
    
    create_performance_ui();
    
    // Create performance update timer
    lv_timer_create(performance_timer_cb, 16, NULL); // ~60fps updates
    
    ESP_LOGI("LVGL_Performance", "Performance demo created successfully");
}

void setup() {
    Serial.begin(115200);
    Serial.println("M5Stack Tab5 LVGL Performance Demo");
    Serial.println("Demonstrating 1280x720 @ 60fps with PPA acceleration");
    
    // Initialize Tab5duino framework
    Tab5duino.begin();
    
    // Initialize LVGL with performance optimized config
    lvgl_tab5_config_t config = LVGL_TAB5_CONFIG_DEFAULT;
    config.enable_ppa_acceleration = true;
    config.enable_vsync = true;
    config.buffer_lines = 120; // Larger buffers for better performance
    config.tick_period_ms = 5;  // 5ms ticks for smooth animations
    
    esp_err_t ret = LVGL.begin(&config);
    if (ret != ESP_OK) {
        Serial.printf("Failed to initialize LVGL: %s\n", esp_err_to_name(ret));
        return;
    }
    
    // Start LVGL
    ret = LVGL.start();
    if (ret != ESP_OK) {
        Serial.printf("Failed to start LVGL: %s\n", esp_err_to_name(ret));
        return;
    }
    
    Serial.println("LVGL performance demo started");
    Serial.println("Watch for smooth 60fps animations with PPA acceleration");
}

void loop() {
    static uint32_t last_stats = 0;
    
    // Print detailed performance stats every 10 seconds
    if (millis() - last_stats > 10000) {
        last_stats = millis();
        
        if (LVGL.isReady()) {
            uint16_t fps;
            uint8_t cpu_usage;
            uint32_t mem_used;
            
            if (LVGL.getPerformanceStats(&fps, &cpu_usage, &mem_used) == ESP_OK) {
                Serial.printf("═══ Performance Stats ═══\n");
                Serial.printf("FPS: %d/60 (%.1f%%)\n", fps, (fps / 60.0f) * 100);
                Serial.printf("CPU Usage: %d%%\n", cpu_usage);
                Serial.printf("Memory Used: %.1f MB\n", mem_used / (1024.0f * 1024.0f));
                Serial.printf("Animated Objects: %d\n", animated_objects.size());
                Serial.printf("Display Brightness: %d/255\n", LVGL.getBrightness());
                
                // Memory breakdown
                multi_heap_info_t heap_info;
                heap_caps_get_info(&heap_info, MALLOC_CAP_INTERNAL);
                Serial.printf("Internal RAM: %lu KB free\n", 
                             heap_info.total_free_bytes / 1024);
                
                heap_caps_get_info(&heap_info, MALLOC_CAP_SPIRAM);
                Serial.printf("PSRAM: %lu KB free\n", 
                             heap_info.total_free_bytes / 1024);
                             
                Serial.printf("════════════════════════════\n");
            }
        }
    }
    
    delay(10);
}