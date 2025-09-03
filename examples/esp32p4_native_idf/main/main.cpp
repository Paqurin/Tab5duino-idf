/**
 * @file main.cpp
 * @brief Native ESP32-P4 ESP-IDF application for M5Stack Tab5
 * 
 * This demonstrates proper ESP32-P4 support using pure ESP-IDF framework
 * with RISC-V architecture and proper bootloader support
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

static const char *TAG = "ESP32P4_NATIVE";

// M5Stack Tab5 GPIO definitions
#define LED_BUILTIN GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_0

extern "C" void app_main(void)
{
    printf("\n==================================================\n");
    printf("ESP32-P4 Native ESP-IDF Application\n");
    printf("M5Stack Tab5 Hardware Platform\n");
    printf("==================================================\n");
    
    // Get chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    // Get flash size
    uint32_t flash_size_mb;
    esp_flash_get_size(NULL, &flash_size_mb);
    flash_size_mb /= (1024 * 1024);
    
    // System information
    ESP_LOGI(TAG, "Chip: ESP32-P4 (RISC-V architecture)");
    ESP_LOGI(TAG, "Silicon Revision: v%d.%d", chip_info.revision / 100, chip_info.revision % 100);
    ESP_LOGI(TAG, "CPU Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Flash Size: %ld MB", flash_size_mb);
    ESP_LOGI(TAG, "Free Heap: %ld KB", esp_get_free_heap_size() / 1024);
    
    // Feature detection
    ESP_LOGI(TAG, "Chip Features:");
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
        ESP_LOGI(TAG, "  - WiFi 6 (802.11 b/g/n)");
    }
    if (chip_info.features & CHIP_FEATURE_BT) {
        ESP_LOGI(TAG, "  - Bluetooth 5.0");
    }
    if (chip_info.features & CHIP_FEATURE_IEEE802154) {
        ESP_LOGI(TAG, "  - IEEE 802.15.4");
    }
    
    // GPIO initialization
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_BUILTIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t gpio_result = gpio_config(&led_config);
    if (gpio_result == ESP_OK) {
        ESP_LOGI(TAG, "LED GPIO %d configured successfully", LED_BUILTIN);
    } else {
        ESP_LOGE(TAG, "LED GPIO configuration failed: %s", esp_err_to_name(gpio_result));
    }
    
    ESP_LOGI(TAG, "System initialized successfully");
    ESP_LOGI(TAG, "Starting main application loop...");
    ESP_LOGI(TAG, "==================================================");
    
    // Main application loop
    bool led_state = false;
    uint32_t loop_count = 0;
    
    while (1) {
        // Toggle LED
        gpio_set_level(LED_BUILTIN, led_state ? 1 : 0);
        led_state = !led_state;
        
        // Status report every 5 seconds
        if (loop_count % 50 == 0) {
            ESP_LOGI(TAG, "[%ld] ESP32-P4 Running - Heap: %ld KB, LED: %s", 
                    loop_count / 10,
                    esp_get_free_heap_size() / 1024,
                    led_state ? "ON" : "OFF");
        }
        
        loop_count++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}