/**
 * @file main.cpp
 * @brief Basic LVGL Example for M5Stack Tab5
 * 
 * This example demonstrates basic LVGL integration with Tab5duino framework.
 * It creates a simple UI with buttons, labels, and basic interactions.
 */

#include <Arduino.h>
#include <Tab5duino.h>
#include <lvgl_tab5duino.h>

// LVGL objects
static lv_obj_t* main_screen;
static lv_obj_t* button1;
static lv_obj_t* button2;
static lv_obj_t* label_status;
static lv_obj_t* slider;
static lv_obj_t* bar;

// Global LVGL instance
extern LVGLTab5duino& LVGL;

// Event callbacks
static void button1_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI("LVGL_Example", "Button 1 clicked!");
        lv_label_set_text(label_status, "Button 1 Clicked!");
        
        // Animate the button
        lv_obj_t* btn = lv_event_get_target(e);
        static lv_style_t style_pressed;
        lv_style_init(&style_pressed);
        lv_style_set_bg_color(&style_pressed, lv_color_hex(0x00AA00));
        lv_obj_add_style(btn, &style_pressed, LV_STATE_DEFAULT);
        
        // Reset after 500ms
        lv_timer_t* timer = lv_timer_create([](lv_timer_t* timer) {
            lv_obj_t* btn = (lv_obj_t*)timer->user_data;
            lv_obj_remove_style_all(btn);
            lv_timer_del(timer);
        }, 500, btn);
    }
}

static void button2_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI("LVGL_Example", "Button 2 clicked!");
        lv_label_set_text(label_status, "Button 2 Clicked!");
        
        // Toggle screen brightness
        static bool bright = true;
        bright = !bright;
        LVGL.setBrightness(bright ? 255 : 128);
    }
}

static void slider_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    
    // Update progress bar
    lv_bar_set_value(bar, value, LV_ANIM_ON);
    
    // Update status label
    static char buf[32];
    snprintf(buf, sizeof(buf), "Slider: %d%%", (int)value);
    lv_label_set_text(label_status, buf);
    
    ESP_LOGI("LVGL_Example", "Slider value: %d", (int)value);
}

void create_main_ui() {
    // Create main screen
    main_screen = LVGL.createScreen();
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x003366), LV_PART_MAIN);
    
    // Create title label
    lv_obj_t* title = lv_label_create(main_screen);
    lv_label_set_text(title, "M5Stack Tab5 LVGL Demo");
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create button 1
    button1 = lv_btn_create(main_screen);
    lv_obj_set_size(button1, 200, 60);
    lv_obj_align(button1, LV_ALIGN_CENTER, -120, -100);
    lv_obj_add_event_cb(button1, button1_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* btn1_label = lv_label_create(button1);
    lv_label_set_text(btn1_label, "Animate");
    lv_obj_center(btn1_label);
    
    // Create button 2
    button2 = lv_btn_create(main_screen);
    lv_obj_set_size(button2, 200, 60);
    lv_obj_align(button2, LV_ALIGN_CENTER, 120, -100);
    lv_obj_add_event_cb(button2, button2_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* btn2_label = lv_label_create(button2);
    lv_label_set_text(btn2_label, "Brightness");
    lv_obj_center(btn2_label);
    
    // Create slider
    slider = lv_slider_create(main_screen);
    lv_obj_set_size(slider, 400, 20);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Create progress bar
    bar = lv_bar_create(main_screen);
    lv_obj_set_size(bar, 400, 30);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 50);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 50, LV_ANIM_OFF);
    
    // Create status label
    label_status = lv_label_create(main_screen);
    lv_label_set_text(label_status, "Ready - Touch controls to interact");
    lv_obj_set_style_text_color(label_status, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Create performance info
    lv_obj_t* perf_label = lv_label_create(main_screen);
    lv_label_set_text(perf_label, "1280x720 @ 60fps - ESP32-P4 + 32MB PSRAM");
    lv_obj_set_style_text_color(perf_label, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_text_font(perf_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(perf_label, LV_ALIGN_BOTTOM_LEFT, 10, -5);
    
    // Load the screen
    LVGL.loadScreen(main_screen);
}

// LVGL ready callback
void on_lvgl_ready() {
    ESP_LOGI("LVGL_Example", "LVGL is ready! Creating UI...");
    
    // Create UI in thread-safe manner
    create_main_ui();
    
    ESP_LOGI("LVGL_Example", "UI created successfully");
}

void setup() {
    Serial.begin(115200);
    Serial.println("M5Stack Tab5 LVGL Basic Example");
    
    // Initialize Tab5duino framework with LVGL enabled
    Tab5duino.begin();
    
    // Initialize LVGL
    esp_err_t ret = LVGL.begin();
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
    
    Serial.println("LVGL started successfully");
    Serial.println("Touch the screen to interact with the UI");
}

void loop() {
    // Tab5duino framework handles LVGL task automatically
    // Your main application code can go here
    
    static uint32_t last_perf_time = 0;
    if (millis() - last_perf_time > 5000) { // Every 5 seconds
        last_perf_time = millis();
        
        if (LVGL.isReady()) {
            uint16_t fps;
            uint8_t cpu_usage;
            uint32_t mem_used;
            
            if (LVGL.getPerformanceStats(&fps, &cpu_usage, &mem_used) == ESP_OK) {
                Serial.printf("Performance - FPS: %d, CPU: %d%%, Memory: %lu KB\n", 
                             fps, cpu_usage, mem_used / 1024);
            }
        }
    }
    
    delay(10);
}