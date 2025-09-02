/*
 * Audio HAL for M5Stack Tab5
 * PDM Microphone Interface with Audio Processing
 */

#ifndef _AUDIO_HAL_H_
#define _AUDIO_HAL_H_

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Audio specifications
#define AUDIO_SAMPLE_RATE_8K        8000
#define AUDIO_SAMPLE_RATE_16K       16000
#define AUDIO_SAMPLE_RATE_22K       22050
#define AUDIO_SAMPLE_RATE_32K       32000
#define AUDIO_SAMPLE_RATE_44K       44100
#define AUDIO_SAMPLE_RATE_48K       48000

#define AUDIO_BITS_PER_SAMPLE_16    16
#define AUDIO_BITS_PER_SAMPLE_24    24
#define AUDIO_BITS_PER_SAMPLE_32    32

#define AUDIO_CHANNELS_MONO         1
#define AUDIO_CHANNELS_STEREO       2

// Audio buffer configuration
#define AUDIO_BUFFER_SIZE_MIN       512
#define AUDIO_BUFFER_SIZE_MAX       8192
#define AUDIO_BUFFER_COUNT_MIN      2
#define AUDIO_BUFFER_COUNT_MAX      8

// Audio format structure
typedef struct {
    uint32_t sample_rate;       // Sample rate in Hz
    uint8_t bits_per_sample;    // Bits per sample (16, 24, 32)
    uint8_t channels;           // Number of channels (1=mono, 2=stereo)
    uint16_t buffer_size;       // Buffer size in samples
    uint8_t buffer_count;       // Number of buffers for DMA
} audio_format_t;

// Audio configuration
typedef struct {
    // Hardware pins
    uint8_t mic_data_pin;       // PDM microphone data pin
    uint8_t mic_clk_pin;        // PDM microphone clock pin
    uint8_t spk_data_pin;       // Speaker data pin (I2S)
    uint8_t spk_bclk_pin;       // Speaker bit clock pin
    uint8_t spk_ws_pin;         // Speaker word select pin
    
    // Input (microphone) configuration
    audio_format_t mic_format;
    bool mic_enable;
    uint8_t mic_gain;           // Microphone gain (0-255)
    
    // Output (speaker) configuration
    audio_format_t spk_format;
    bool spk_enable;
    uint8_t spk_volume;         // Speaker volume (0-255)
    
    // Processing configuration
    bool enable_aec;            // Acoustic Echo Cancellation
    bool enable_ns;             // Noise Suppression
    bool enable_agc;            // Automatic Gain Control
    bool enable_vad;            // Voice Activity Detection
} audio_config_t;

// Audio event types
typedef enum {
    AUDIO_EVENT_NONE = 0,
    AUDIO_EVENT_MIC_DATA_READY,
    AUDIO_EVENT_SPK_BUFFER_EMPTY,
    AUDIO_EVENT_VOICE_DETECTED,
    AUDIO_EVENT_VOICE_END,
    AUDIO_EVENT_CLIPPING,
    AUDIO_EVENT_UNDERRUN,
    AUDIO_EVENT_OVERRUN,
    AUDIO_EVENT_ERROR
} audio_event_type_t;

// Audio event data
typedef struct {
    audio_event_type_t type;
    void* data;                 // Event-specific data
    size_t data_size;           // Size of event data
    uint64_t timestamp_us;      // Event timestamp
} audio_event_t;

// Audio buffer structure
typedef struct {
    int16_t* data;              // Audio data buffer
    size_t size;                // Buffer size in bytes
    size_t sample_count;        // Number of samples
    uint8_t channels;           // Number of channels
    uint64_t timestamp_us;      // Buffer timestamp
    bool is_valid;              // Buffer validity flag
} audio_buffer_t;

// Audio statistics
typedef struct {
    uint32_t samples_processed;
    uint32_t buffer_overruns;
    uint32_t buffer_underruns;
    uint32_t clipping_events;
    float peak_level;           // Peak level in dB
    float rms_level;            // RMS level in dB
    float snr_db;               // Signal-to-noise ratio
    bool voice_detected;        // Current voice activity
} audio_stats_t;

// Audio handle
typedef struct audio_handle_s* audio_handle_t;

// Audio callback types
typedef void (*audio_input_callback_t)(audio_handle_t handle,
                                      const audio_buffer_t* buffer,
                                      void* user_data);

typedef void (*audio_output_callback_t)(audio_handle_t handle,
                                       audio_buffer_t* buffer,
                                       void* user_data);

typedef void (*audio_event_callback_t)(audio_handle_t handle,
                                      const audio_event_t* event,
                                      void* user_data);

// Audio initialization and management
esp_err_t audio_hal_init(const audio_config_t* config, audio_handle_t* handle);
esp_err_t audio_hal_deinit(audio_handle_t handle);
esp_err_t audio_hal_start(audio_handle_t handle);
esp_err_t audio_hal_stop(audio_handle_t handle);

// Microphone functions
esp_err_t audio_hal_mic_start(audio_handle_t handle);
esp_err_t audio_hal_mic_stop(audio_handle_t handle);
esp_err_t audio_hal_mic_read(audio_handle_t handle, audio_buffer_t* buffer, uint32_t timeout_ms);
esp_err_t audio_hal_mic_set_gain(audio_handle_t handle, uint8_t gain);
uint8_t audio_hal_mic_get_gain(audio_handle_t handle);

// Speaker functions
esp_err_t audio_hal_spk_start(audio_handle_t handle);
esp_err_t audio_hal_spk_stop(audio_handle_t handle);
esp_err_t audio_hal_spk_write(audio_handle_t handle, const audio_buffer_t* buffer, uint32_t timeout_ms);
esp_err_t audio_hal_spk_set_volume(audio_handle_t handle, uint8_t volume);
uint8_t audio_hal_spk_get_volume(audio_handle_t handle);

// Audio processing functions
esp_err_t audio_hal_enable_aec(audio_handle_t handle, bool enable);
esp_err_t audio_hal_enable_ns(audio_handle_t handle, bool enable);
esp_err_t audio_hal_enable_agc(audio_handle_t handle, bool enable);
esp_err_t audio_hal_enable_vad(audio_handle_t handle, bool enable);

// Voice Activity Detection
esp_err_t audio_hal_vad_set_threshold(audio_handle_t handle, float threshold);
float audio_hal_vad_get_threshold(audio_handle_t handle);
bool audio_hal_vad_is_voice_detected(audio_handle_t handle);

// Audio analysis functions
esp_err_t audio_hal_get_peak_level(audio_handle_t handle, float* level_db);
esp_err_t audio_hal_get_rms_level(audio_handle_t handle, float* level_db);
esp_err_t audio_hal_get_spectrum(audio_handle_t handle, float* spectrum, size_t bins);
esp_err_t audio_hal_get_stats(audio_handle_t handle, audio_stats_t* stats);

// Callback registration
esp_err_t audio_hal_register_input_callback(audio_handle_t handle,
                                           audio_input_callback_t callback,
                                           void* user_data);
esp_err_t audio_hal_register_output_callback(audio_handle_t handle,
                                            audio_output_callback_t callback,
                                            void* user_data);
esp_err_t audio_hal_register_event_callback(audio_handle_t handle,
                                           audio_event_callback_t callback,
                                           void* user_data);

// Information functions
esp_err_t audio_hal_get_info(audio_handle_t handle, audio_config_t* info);
bool audio_hal_is_ready(audio_handle_t handle);

// Utility functions
static inline float audio_sample_to_db(int16_t sample) {
    if (sample == 0) return -96.0f;  // Silence floor
    return 20.0f * log10f(abs(sample) / 32767.0f);
}

static inline int16_t audio_db_to_sample(float db) {
    if (db <= -96.0f) return 0;
    return (int16_t)(32767.0f * powf(10.0f, db / 20.0f));
}

static inline void audio_apply_gain(int16_t* buffer, size_t samples, float gain) {
    for (size_t i = 0; i < samples; i++) {
        float sample = buffer[i] * gain;
        if (sample > 32767.0f) sample = 32767.0f;
        if (sample < -32768.0f) sample = -32768.0f;
        buffer[i] = (int16_t)sample;
    }
}

static inline float audio_calculate_rms(const int16_t* buffer, size_t samples) {
    if (!buffer || samples == 0) return 0.0f;
    float sum = 0.0f;
    for (size_t i = 0; i < samples; i++) {
        float sample = buffer[i] / 32767.0f;
        sum += sample * sample;
    }
    return sqrtf(sum / samples);
}

// Default configurations
extern const audio_config_t AUDIO_CONFIG_DEFAULT;
extern const audio_format_t AUDIO_FORMAT_16K_MONO;
extern const audio_format_t AUDIO_FORMAT_44K_STEREO;

#ifdef __cplusplus
}
#endif

#endif // _AUDIO_HAL_H_