"""
Tab5duino-IDF Framework Builder for PlatformIO
Handles compilation and linking of Arduino-compatible code with ESP-IDF
"""

import os
import sys
from os.path import isdir, join, realpath
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

# Framework configuration
FRAMEWORK_DIR = platform.get_package_dir("framework-tab5duino-idf")
FRAMEWORK_VERSION = "1.0.0"

print("Tab5duino-IDF Framework v{} - ESP32-P4 M5Stack Tab5".format(FRAMEWORK_VERSION))

# Add framework paths
env.Prepend(
    CPPPATH=[
        join(FRAMEWORK_DIR, "cores", "tab5duino"),
        join(FRAMEWORK_DIR, "cores", "tab5duino", "hal"),
        join(FRAMEWORK_DIR, "libraries"),
        join(FRAMEWORK_DIR, "variants", env.BoardConfig().get("build.variant", ""))
    ]
)

# Compiler flags for ESP32-P4 RISC-V
env.Append(
    CPPDEFINES=[
        "ARDUINO_ARCH_ESP32",
        "ESP32",
        "ESP32P4",
        "ARDUINO_BOARD=\\\"{}\\\"".format(env.BoardConfig().get("build.variant", "").upper()),
        ("F_CPU", board.get("build.f_cpu", "400000000L")),
        ("ARDUINO", 10812),
        ("TAB5DUINO_FRAMEWORK", "1"),
        "__RISC_V__",
    ],

    CFLAGS=[
        "-std=gnu17"
    ],

    CXXFLAGS=[
        "-std=gnu++17",
        "-fno-rtti",
        "-fno-exceptions"
    ],

    CCFLAGS=[
        "-Os",  # Optimize for size
        "-march=rv32imafc",
        "-mabi=ilp32f",
        "-ffunction-sections",
        "-fdata-sections",
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter",
        "-Wno-unused-function",
        "-Wno-unused-variable",
        "-Wno-deprecated-declarations",
        "-Wno-missing-field-initializers",
        "-Wno-sign-compare"
    ],

    LINKFLAGS=[
        "-Os",
        "-march=rv32imafc", 
        "-mabi=ilp32f",
        "-Wl,--gc-sections",
        "-Wl,--cref",
        "-Wl,--check-sections",
        "-Wl,--unresolved-symbols=report-all",
        "-Wl,--warn-common",
        "-Wl,--warn-section-align"
    ]
)

# Memory configuration for ESP32-P4
env.Append(
    LINKFLAGS=[
        "-Wl,--defsym=_start=0x42000000",
        "-Wl,--defsym=_heap_start=0x50000000"
    ]
)

# Enable PSRAM support if configured
if board.get("build.psram_type", ""):
    env.Append(
        CPPDEFINES=[
            "BOARD_HAS_PSRAM",
            ("CONFIG_SPIRAM_SUPPORT", 1),
            ("CONFIG_SPIRAM_USE_CAPS_ALLOC", 1)
        ]
    )

# Add libraries path if exists
libs_dir = join(FRAMEWORK_DIR, "libraries")
if isdir(libs_dir):
    env.Append(LIBSOURCE_DIRS=[libs_dir])

# Framework core sources
core_src = []
core_dir = join(FRAMEWORK_DIR, "cores", "tab5duino")

if isdir(core_dir):
    for root, dirs, files in os.walk(core_dir):
        for file in files:
            if file.endswith(('.c', '.cpp', '.S')):
                core_src.append(join(root, file))

# HAL sources  
hal_dir = join(core_dir, "hal")
if isdir(hal_dir):
    for root, dirs, files in os.walk(hal_dir):
        for file in files:
            if file.endswith(('.c', '.cpp', '.S')):
                core_src.append(join(root, file))

# Build framework library
if core_src:
    env.BuildLibrary(
        join("$BUILD_DIR", "FrameworkTab5duino"),
        core_src
    )

# Variant configuration
variant_dir = join(FRAMEWORK_DIR, "variants", board.get("build.variant", ""))
if isdir(variant_dir):
    env.Append(CPPPATH=[variant_dir])
    
    # Add variant-specific sources
    variant_src = []
    for root, dirs, files in os.walk(variant_dir):
        for file in files:
            if file.endswith(('.c', '.cpp', '.S')):
                variant_src.append(join(root, file))
    
    if variant_src:
        env.BuildLibrary(
            join("$BUILD_DIR", "FrameworkVariant"),  
            variant_src
        )

# ESP-IDF integration
env.Append(
    CPPDEFINES=[
        "ESP_PLATFORM",
        ("IDF_VER", "v5.3-dev")
    ]
)

# Add ESP-IDF include paths (these would be provided by ESP-IDF platform)
esp_idf_includes = [
    "components/esp_common/include",
    "components/esp_system/include", 
    "components/freertos/FreeRTOS-Kernel/include",
    "components/freertos/esp_additions/include",
    "components/driver/include",
    "components/hal/esp32p4/include",
    "components/hal/include",
    "components/soc/esp32p4/include",
    "components/soc/include",
    "components/log/include",
    "components/esp_timer/include",
    "components/esp_pm/include"
]

# Print build information
print("Framework Directory: {}".format(FRAMEWORK_DIR))
print("Core Sources: {} files".format(len(core_src)))
print("Target MCU: {}".format(board.get("build.mcu", "esp32p4")))
print("CPU Frequency: {} MHz".format(int(board.get("build.f_cpu", "400000000")) // 1000000))
print("Flash Size: {}".format(board.get("upload.flash_size", "16MB")))
if board.get("build.psram_type", ""):
    print("PSRAM: Enabled ({} MB)".format(board.get("build.psram_size", "32MB")))

# Framework ready indicator
env["FRAMEWORK_TAB5DUINO"] = True