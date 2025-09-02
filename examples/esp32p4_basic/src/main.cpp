/**
 * ESP32-P4 Basic Test for M5Stack Tab5
 * Uses ESP-IDF framework for better ESP32-P4 compatibility
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "M5Tab5_ESP32P4";

// GPIO configuration for M5Stack Tab5
#define LED_GPIO GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_0

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "M5Stack Tab5 - ESP32-P4 Test Starting");
    ESP_LOGI(TAG, "Chip: %s, Revision: %d", CONFIG_IDF_TARGET, esp_chip_info().revision);
    ESP_LOGI(TAG, "Free heap: %ld KB", esp_get_free_heap_size() / 1024);
    ESP_LOGI(TAG, "Flash size: %d MB", spi_flash_get_chip_size() / (1024 * 1024));
    
    // Configure LED GPIO
    gpio_config_t led_config = {};
    led_config.pin_bit_mask = (1ULL << LED_GPIO);
    led_config.mode = GPIO_MODE_OUTPUT;
    led_config.pull_up_en = GPIO_PULLUP_DISABLE;
    led_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    led_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&led_config);
    
    ESP_LOGI(TAG, "GPIO initialized successfully");
    
    // Main loop
    bool led_state = false;
    int counter = 0;
    
    while (1) {
        // Toggle LED
        gpio_set_level(LED_GPIO, led_state);
        led_state = !led_state;
        
        // Print status every 5 seconds
        if (counter % 50 == 0) {
            ESP_LOGI(TAG, "[%d] ESP32-P4 alive - Free heap: %ld KB", 
                    counter / 10, esp_get_free_heap_size() / 1024);
        }
        
        counter++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}