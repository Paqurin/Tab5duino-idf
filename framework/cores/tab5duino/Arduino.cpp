/*
 * Arduino.cpp - Arduino Compatibility Layer Implementation
 * Provides Arduino-style API on ESP-IDF foundation
 */

#include "Arduino.h"
#include "Tab5duino.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_random.h"
#include <stdlib.h>

static const char* TAG = "Arduino";

// GPIO configuration storage
static bool gpio_initialized[GPIO_NUM_MAX] = {false};

// Random seed storage
static unsigned long random_seed = 1;

// Main function - entry point for Arduino applications
int main(void) {
    // Initialize Tab5duino framework with default config
    esp_err_t ret = tab5duino_init(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Tab5duino framework: %s", esp_err_to_name(ret));
        return -1;
    }

    // Start the framework (this will call setup() and loop())
    ret = tab5duino_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Tab5duino framework: %s", esp_err_to_name(ret));
        return -1;
    }

    // Framework runs in its own task, so we can return here
    return 0;
}

// GPIO Functions
void pinMode(uint8_t pin, uint8_t mode) {
    if (pin >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid pin number: %d", pin);
        return;
    }

    gpio_config_t config = {0};
    config.pin_bit_mask = (1ULL << pin);
    config.intr_type = GPIO_INTR_DISABLE;

    switch (mode) {
        case INPUT:
            config.mode = GPIO_MODE_INPUT;
            config.pull_up_en = GPIO_PULLUP_DISABLE;
            config.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        
        case INPUT_PULLUP:
            config.mode = GPIO_MODE_INPUT;
            config.pull_up_en = GPIO_PULLUP_ENABLE;
            config.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        
        case INPUT_PULLDOWN:
            config.mode = GPIO_MODE_INPUT;
            config.pull_up_en = GPIO_PULLUP_DISABLE;
            config.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        
        case OUTPUT:
            config.mode = GPIO_MODE_OUTPUT;
            config.pull_up_en = GPIO_PULLUP_DISABLE;
            config.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        
        case OUTPUT_OPEN_DRAIN:
            config.mode = GPIO_MODE_OUTPUT_OD;
            config.pull_up_en = GPIO_PULLUP_ENABLE;  // Open drain typically needs pullup
            config.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        
        default:
            ESP_LOGE(TAG, "Invalid pin mode: %d", mode);
            return;
    }

    esp_err_t ret = gpio_config(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", pin, esp_err_to_name(ret));
        return;
    }

    gpio_initialized[pin] = true;
}

void digitalWrite(uint8_t pin, uint8_t val) {
    if (pin >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid pin number: %d", pin);
        return;
    }

    if (!gpio_initialized[pin]) {
        ESP_LOGW(TAG, "GPIO %d not initialized, setting as output", pin);
        pinMode(pin, OUTPUT);
    }

    esp_err_t ret = gpio_set_level((gpio_num_t)pin, val ? 1 : 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO %d level: %s", pin, esp_err_to_name(ret));
    }
}

int digitalRead(uint8_t pin) {
    if (pin >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid pin number: %d", pin);
        return LOW;
    }

    if (!gpio_initialized[pin]) {
        ESP_LOGW(TAG, "GPIO %d not initialized, setting as input", pin);
        pinMode(pin, INPUT);
    }

    return gpio_get_level((gpio_num_t)pin);
}

int analogRead(uint8_t pin) {
    // Placeholder for ADC implementation
    ESP_LOGW(TAG, "analogRead not yet implemented for pin %d", pin);
    return 0;
}

void analogWrite(uint8_t pin, int val) {
    // Placeholder for PWM/DAC implementation
    ESP_LOGW(TAG, "analogWrite not yet implemented for pin %d, value %d", pin, val);
}

// Timing Functions
unsigned long millis(void) {
    return (unsigned long)(esp_timer_get_time() / 1000);
}

unsigned long micros(void) {
    return (unsigned long)esp_timer_get_time();
}

void delay(unsigned long ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void delayMicroseconds(unsigned int us) {
    if (us < 10000) {
        // For short delays, use busy wait
        int64_t start = esp_timer_get_time();
        while ((esp_timer_get_time() - start) < us) {
            // Busy wait
        }
    } else {
        // For longer delays, use vTaskDelay
        vTaskDelay(pdMS_TO_TICKS(us / 1000));
    }
}

// Mathematical Functions
long map(long value, long fromLow, long fromHigh, long toLow, long toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

double mapf(double value, double fromLow, double fromHigh, double toLow, double toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

// Random Functions
void randomSeed(unsigned long seed) {
    random_seed = seed;
    srand(seed);
}

long random(long max) {
    if (max <= 0) return 0;
    return esp_random() % max;
}

long random(long min, long max) {
    if (min >= max) return min;
    return min + (esp_random() % (max - min));
}

// String Functions
int sprintf_P(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(str, format, args);
    va_end(args);
    return ret;
}

int snprintf_P(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(str, size, format, args);
    va_end(args);
    return ret;
}

// Interrupt Functions
static interrupt_handler_t interrupt_handlers[GPIO_NUM_MAX] = {NULL};

static void IRAM_ATTR gpio_interrupt_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    if (gpio_num < GPIO_NUM_MAX && interrupt_handlers[gpio_num] != NULL) {
        interrupt_handlers[gpio_num]();
    }
}

void attachInterrupt(uint8_t pin, interrupt_handler_t handler, int mode) {
    if (pin >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid pin number: %d", pin);
        return;
    }

    if (!gpio_initialized[pin]) {
        ESP_LOGW(TAG, "GPIO %d not initialized, setting as input", pin);
        pinMode(pin, INPUT);
    }

    // Store the handler
    interrupt_handlers[pin] = handler;

    // Configure interrupt
    gpio_set_intr_type((gpio_num_t)pin, (gpio_int_type_t)mode);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add((gpio_num_t)pin, gpio_interrupt_handler, (void*)(uint32_t)pin);
}

void detachInterrupt(uint8_t pin) {
    if (pin >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid pin number: %d", pin);
        return;
    }

    gpio_isr_handler_remove((gpio_num_t)pin);
    interrupt_handlers[pin] = NULL;
    gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_DISABLE);
}

// Pulse Functions
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    return pulseInLong(pin, state, timeout);
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
    if (pin >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid pin number: %d", pin);
        return 0;
    }

    if (!gpio_initialized[pin]) {
        ESP_LOGW(TAG, "GPIO %d not initialized, setting as input", pin);
        pinMode(pin, INPUT);
    }

    int64_t start_time = esp_timer_get_time();
    int64_t timeout_us = timeout;
    
    // Wait for the pulse to start
    while (digitalRead(pin) != state) {
        if ((esp_timer_get_time() - start_time) > timeout_us) {
            return 0; // Timeout
        }
    }
    
    // Measure pulse duration
    int64_t pulse_start = esp_timer_get_time();
    while (digitalRead(pin) == state) {
        if ((esp_timer_get_time() - start_time) > timeout_us) {
            return 0; // Timeout
        }
    }
    
    return (unsigned long)(esp_timer_get_time() - pulse_start);
}

// Shift Functions
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
    uint8_t value = 0;
    
    if (!gpio_initialized[dataPin]) {
        pinMode(dataPin, INPUT);
    }
    if (!gpio_initialized[clockPin]) {
        pinMode(clockPin, OUTPUT);
    }

    for (uint8_t i = 0; i < 8; ++i) {
        digitalWrite(clockPin, HIGH);
        if (bitOrder == LSBFIRST) {
            value |= digitalRead(dataPin) << i;
        } else {
            value |= digitalRead(dataPin) << (7 - i);
        }
        digitalWrite(clockPin, LOW);
    }
    return value;
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
    if (!gpio_initialized[dataPin]) {
        pinMode(dataPin, OUTPUT);
    }
    if (!gpio_initialized[clockPin]) {
        pinMode(clockPin, OUTPUT);
    }

    for (uint8_t i = 0; i < 8; i++) {
        if (bitOrder == LSBFIRST) {
            digitalWrite(dataPin, !!(val & (1 << i)));
        } else {
            digitalWrite(dataPin, !!(val & (1 << (7 - i))));
        }
        
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}

// Tone Functions (placeholder implementation)
void tone(uint8_t pin, unsigned int frequency, unsigned long duration) {
    ESP_LOGW(TAG, "tone() not yet implemented for pin %d, freq %d, duration %lu", 
             pin, frequency, duration);
}

void noTone(uint8_t pin) {
    ESP_LOGW(TAG, "noTone() not yet implemented for pin %d", pin);
}