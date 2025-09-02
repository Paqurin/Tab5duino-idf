/*
 * Arduino.h - Arduino Compatibility Layer for Tab5duino Framework
 * Provides Arduino-style API on ESP-IDF foundation
 */

#ifndef _ARDUINO_H_
#define _ARDUINO_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "esp_system.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

// Arduino constants
#define HIGH    1
#define LOW     0

#define INPUT               GPIO_MODE_INPUT
#define OUTPUT              GPIO_MODE_OUTPUT
#define INPUT_PULLUP        GPIO_MODE_INPUT
#define INPUT_PULLDOWN      GPIO_MODE_INPUT
#define OUTPUT_OPEN_DRAIN   GPIO_MODE_OUTPUT_OD

#define RISING    GPIO_INTR_POSEDGE
#define FALLING   GPIO_INTR_NEGEDGE
#define CHANGE    GPIO_INTR_ANYEDGE

// Arduino data types
typedef bool boolean;
typedef uint8_t byte;
typedef uint16_t word;

// Mathematical constants
#ifndef PI
#define PI          3.1415926535897932384626433832795
#endif
#ifndef HALF_PI
#define HALF_PI     1.5707963267948966192313216916398
#endif
#ifndef TWO_PI
#define TWO_PI      6.283185307179586476925286766559
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD  0.017453292519943295769236907684886
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG  57.295779513082320876798154814105
#endif
#ifndef EULER
#define EULER       2.718281828459045235360287471352
#endif

// Useful macros
#define min(a,b)                ((a)<(b)?(a):(b))
#define max(a,b)                ((a)>(b)?(a):(b))
#define abs(x)                  ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)                ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg)            ((deg)*DEG_TO_RAD)
#define degrees(rad)            ((rad)*RAD_TO_DEG)
#define sq(x)                   ((x)*(x))

// Bit manipulation macros
#define lowByte(w)              ((uint8_t) ((w) & 0xff))
#define highByte(w)             ((uint8_t) ((w) >> 8))
#define bitRead(value, bit)     (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)      ((value) |= (1UL << (bit)))
#define bitClear(value, bit)    ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b)                  (1UL << (b))

// GPIO pin definitions for M5Stack Tab5
#define LED_BUILTIN     2
#define BUTTON_A        0   // Boot button
#define BUTTON_B        46  // GPIO46
#define BUTTON_C        45  // GPIO45

// Display pins
#define TFT_CS          10
#define TFT_DC          11
#define TFT_RST         12
#define TFT_BACKLIGHT   13

// Touch pins
#define TOUCH_SDA       6
#define TOUCH_SCL       7
#define TOUCH_INT       8
#define TOUCH_RST       9

// IMU pins
#define IMU_SDA         4
#define IMU_SCL         5
#define IMU_INT1        14
#define IMU_INT2        15

// Audio pins
#define MIC_DATA        16
#define MIC_CLK         17
#define SPK_DATA        18
#define SPK_BCLK        19
#define SPK_WS          20

// USB pins
#define USB_DM          26
#define USB_DP          27

// Arduino core functions
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);

// Timing functions
unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

// Mathematical functions
long map(long value, long fromLow, long fromHigh, long toLow, long toHigh);
double mapf(double value, double fromLow, double fromHigh, double toLow, double toHigh);

// Random functions
void randomSeed(unsigned long seed);
long random(long max);
long random(long min, long max);

// String functions
int sprintf_P(char *str, const char *format, ...);
int snprintf_P(char *str, size_t size, const char *format, ...);

// Interrupt functions
typedef void (*interrupt_handler_t)(void);
void attachInterrupt(uint8_t pin, interrupt_handler_t handler, int mode);
void detachInterrupt(uint8_t pin);

// Pulse functions
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout);

// Shift functions
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);

// Bit order constants
#define LSBFIRST 0
#define MSBFIRST 1

// Tone functions (placeholder - would need PWM implementation)
void tone(uint8_t pin, unsigned int frequency, unsigned long duration);
void noTone(uint8_t pin);

// Arduino main function (implemented by Tab5duino framework)
int main(void);

#ifdef __cplusplus
}

// C++ Arduino classes and functions
#include "WString.h"
#include "Stream.h"
#include "Print.h"
#include "Printable.h"

// Forward declarations of Arduino classes
class HardwareSerial;
class SoftwareSerial;

// Global Serial instances
extern HardwareSerial Serial;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;

// Arduino String class
using String = arduino::WString;

#endif // __cplusplus

#endif // _ARDUINO_H_