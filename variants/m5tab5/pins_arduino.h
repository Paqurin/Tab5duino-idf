/*
 * Pin definitions for M5Stack Tab5 (ESP32-P4)
 * Hardware-specific pin mappings and configurations
 */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// Board identification
#define ARDUINO_M5TAB5
#define ARDUINO_BOARD "M5Stack Tab5"
#define ARDUINO_VARIANT "m5tab5"

// CPU and hardware specifications
static const uint8_t LED_BUILTIN = 2;
#define BUILTIN_LED LED_BUILTIN
#define LED_BUILTIN_ON 1
#define LED_BUILTIN_OFF 0

// Button pins
static const uint8_t BUTTON_A = 0;   // Boot button
static const uint8_t BUTTON_B = 46;  // GPIO46 
static const uint8_t BUTTON_C = 45;  // GPIO45

// Display pins (MIPI-DSI)
static const uint8_t TFT_CS = 10;
static const uint8_t TFT_DC = 11;
static const uint8_t TFT_RST = 12;
static const uint8_t TFT_BACKLIGHT = 13;

// Touch controller pins (GT911)
static const uint8_t TOUCH_SDA = 6;
static const uint8_t TOUCH_SCL = 7;
static const uint8_t TOUCH_INT = 8;
static const uint8_t TOUCH_RST = 9;

// IMU pins (BMI270)
static const uint8_t IMU_SDA = 4;
static const uint8_t IMU_SCL = 5;
static const uint8_t IMU_INT1 = 14;
static const uint8_t IMU_INT2 = 15;

// Audio pins
static const uint8_t MIC_DATA = 16;   // PDM microphone data
static const uint8_t MIC_CLK = 17;    // PDM microphone clock
static const uint8_t SPK_DATA = 18;   // Speaker I2S data
static const uint8_t SPK_BCLK = 19;   // Speaker bit clock
static const uint8_t SPK_WS = 20;     // Speaker word select

// USB OTG pins
static const uint8_t USB_DM = 26;     // USB D- (differential pair)
static const uint8_t USB_DP = 27;     // USB D+ (differential pair)

// Power management pins
static const uint8_t BAT_ADC = 1;     // Battery voltage ADC
static const uint8_t CHG_STAT = 21;   // Charging status
static const uint8_t PWR_EN = 22;     // Power enable

// Solar panel pins
static const uint8_t SOLAR_ADC = 2;   // Solar panel voltage
static const uint8_t SOLAR_EN = 23;   // Solar panel enable

// Expansion connector pins
static const uint8_t EXP_SDA = 35;    // Expansion I2C SDA
static const uint8_t EXP_SCL = 36;    // Expansion I2C SCL
static const uint8_t EXP_TX = 37;     // Expansion UART TX
static const uint8_t EXP_RX = 38;     // Expansion UART RX
static const uint8_t EXP_GPIO1 = 39;  // Expansion GPIO 1
static const uint8_t EXP_GPIO2 = 40;  // Expansion GPIO 2
static const uint8_t EXP_GPIO3 = 41;  // Expansion GPIO 3
static const uint8_t EXP_GPIO4 = 42;  // Expansion GPIO 4

// SPI pins
static const uint8_t SS = 10;         // SPI Slave Select (same as TFT_CS)
static const uint8_t MOSI = 47;       // SPI Master Out Slave In
static const uint8_t MISO = 48;       // SPI Master In Slave Out
static const uint8_t SCK = 49;        // SPI Serial Clock

// I2C pins (primary)
static const uint8_t SDA = IMU_SDA;   // I2C Serial Data
static const uint8_t SCL = IMU_SCL;   // I2C Serial Clock

// UART pins (USB Serial)
static const uint8_t RX = 24;         // USB Serial RX
static const uint8_t TX = 25;         // USB Serial TX

// ADC pins
static const uint8_t A0 = BAT_ADC;    // Analog input 0 (Battery)
static const uint8_t A1 = SOLAR_ADC;  // Analog input 1 (Solar)
static const uint8_t A2 = 28;         // Analog input 2
static const uint8_t A3 = 29;         // Analog input 3
static const uint8_t A4 = 30;         // Analog input 4
static const uint8_t A5 = 31;         // Analog input 5

// DAC pins  
static const uint8_t DAC1 = 32;       // DAC output 1
static const uint8_t DAC2 = 33;       // DAC output 2

// PWM pins (can be used on most GPIO pins)
#define PWM0 LED_BUILTIN
#define PWM1 13  // TFT_BACKLIGHT
#define PWM2 18  // SPK_DATA
#define PWM3 22  // PWR_EN

// Pin count definitions
#define PINS_COUNT          50
#define NUM_DIGITAL_PINS    50
#define NUM_ANALOG_INPUTS   6

// ADC resolution
#define analogInputToDigitalPin(p) (((p) < 6) ? (p) + A0 : -1)
#define digitalPinToInterrupt(p) (((p) < NUM_DIGITAL_PINS) ? (p) : -1)
#define digitalPinHasPWM(p) (p < NUM_DIGITAL_PINS)

// Hardware capabilities
#define BOARD_HAS_PSRAM
#define PSRAM_SIZE (32 * 1024 * 1024)  // 32MB PSRAM

#define FLASH_SIZE (16 * 1024 * 1024)  // 16MB Flash

// Display specifications
#define DISPLAY_WIDTH  1280
#define DISPLAY_HEIGHT 720
#define DISPLAY_PIXEL_FORMAT RGB565

// Touch specifications
#define TOUCH_MAX_POINTS 10
#define TOUCH_I2C_ADDR   0x5D

// IMU specifications  
#define IMU_I2C_ADDR     0x68

// Audio specifications
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_CHANNELS    2

// Power specifications
#define BATTERY_CAPACITY_MAH 5000
#define SOLAR_MAX_POWER_MW   2000

// Hardware feature flags
#define HAS_DISPLAY     1
#define HAS_TOUCH       1
#define HAS_IMU         1
#define HAS_AUDIO       1
#define HAS_WIFI        1
#define HAS_BLUETOOTH   1
#define HAS_USB_OTG     1
#define HAS_SOLAR       1
#define HAS_BATTERY     1

#endif /* Pins_Arduino_h */