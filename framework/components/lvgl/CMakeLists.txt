# LVGL Component for ESP-IDF
# Optimized for M5Stack Tab5 ESP32-P4

cmake_minimum_required(VERSION 3.5)

# Find all LVGL source files
file(GLOB_RECURSE LVGL_SOURCES
    "src/core/*.c"
    "src/draw/*.c"
    "src/font/*.c"
    "src/hal/*.c" 
    "src/misc/*.c"
    "src/widgets/*.c"
    "src/extra/layouts/*.c"
    "src/extra/libs/*.c"
    "src/extra/others/*.c"
    "src/extra/themes/*.c"
    "src/extra/widgets/*.c"
)

# Tab5duino integration sources
set(TAB5_INTEGRATION_SOURCES
    "../../../libraries/LVGL/lvgl_tab5duino.cpp"
)

# Combine all sources
set(COMPONENT_SRCS
    ${LVGL_SOURCES}
    ${TAB5_INTEGRATION_SOURCES}
)

# Include directories
set(COMPONENT_INCLUDE_DIRS
    "."
    "src"
    "src/core"
    "src/draw"
    "src/font"
    "src/hal"
    "src/misc"
    "src/widgets"
    "src/extra"
    "src/extra/layouts"
    "src/extra/libs"  
    "src/extra/others"
    "src/extra/themes"
    "src/extra/widgets"
    "../../../libraries/LVGL"
)

# Public include directories (available to other components)
set(COMPONENT_PUBLIC_INCLUDEDIRS ${COMPONENT_INCLUDE_DIRS})

# Component requirements
set(COMPONENT_REQUIRES
    "driver"
    "esp_timer"
    "esp_common"
    "freertos"
    "heap"
    "log"
    "spi_flash"
)

# Private requirements
set(COMPONENT_PRIV_REQUIRES
    "esp_psram"
)

# Register component
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_INCLUDE_DIRS}
    REQUIRES ${COMPONENT_REQUIRES}
    PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES}
)

# Compiler flags for optimization
target_compile_options(${COMPONENT_LIB} PRIVATE
    -O2                             # Optimize for speed
    -ffunction-sections             # Enable function sections for dead code elimination
    -fdata-sections                 # Enable data sections for dead code elimination
    -Wno-unused-parameter           # Suppress unused parameter warnings
    -Wno-unused-variable            # Suppress unused variable warnings
)

# Linker flags
target_link_libraries(${COMPONENT_LIB} INTERFACE
    -Wl,--gc-sections              # Enable dead code elimination
)

# Preprocessor definitions
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    LV_CONF_INCLUDE_SIMPLE=1
    LV_CONF_PATH="lv_conf.h"
    ESP32_P4_TAB5_OPTIMIZED=1
)

# ESP32-P4 specific optimizations
if(CONFIG_IDF_TARGET_ESP32P4)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        LV_USE_ESP32P4_PPA=1
        LV_ESP32P4_PSRAM_OPTIMIZATION=1
        LV_ESP32P4_HIGH_RESOLUTION=1
    )
    
    # Enable PPA acceleration if available
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -DLVGL_ESP32P4_PPA_ACCELERATION=1
    )
endif()

# Performance monitoring in debug builds
if(CONFIG_COMPILER_OPTIMIZATION_DEBUG)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        LV_USE_PERF_MONITOR=1
        LV_USE_MEM_MONITOR=1
    )
endif()

# Memory optimization for release builds
if(CONFIG_COMPILER_OPTIMIZATION_SIZE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        LV_IMG_CACHE_DEF_SIZE=0
        LV_GRADIENT_MAX_STOPS=2
        LV_GRAD_CACHE_DEF_SIZE=0
    )
endif()