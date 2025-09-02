"""
ESP32-P4 Upload Script for Tab5duino-IDF Framework
Fixes PlatformIO upload issues with ESP32-P4 hardware
"""

import os
import subprocess
import sys
from SCons.Script import Import, Return

Import("env")

def upload_using_esptool(source, target, env):
    """Upload firmware to ESP32-P4 using esptool with correct chip parameter"""
    
    upload_port = env.subst("$UPLOAD_PORT")
    upload_speed = env.subst("$UPLOAD_SPEED") 
    firmware_path = str(source[0])
    
    # ESP32-P4 specific esptool command
    cmd = [
        "esptool.py",
        "--chip", "esp32p4",
        "--port", upload_port,
        "--baud", upload_speed,
        "write_flash",
        "--flash_mode", "qio",
        "--flash_freq", "80m", 
        "--flash_size", "16MB",
        "0x0000", firmware_path
    ]
    
    print("Uploading firmware to ESP32-P4 M5Stack Tab5...")
    print("Command:", " ".join(cmd))
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("Upload successful!")
        print(result.stdout)
        return 0
    except subprocess.CalledProcessError as e:
        print(f"Upload failed: {e}")
        print(f"Error output: {e.stderr}")
        return 1
    except FileNotFoundError:
        print("Error: esptool.py not found. Please install esptool.")
        return 1

# Override the default upload command for ESP32-P4
if env.get("BOARD") == "m5tab5_p4" or "esp32p4" in env.get("BOARD_MCU", ""):
    env.Replace(UPLOADCMD=upload_using_esptool)
    print("ESP32-P4 upload override enabled for Tab5duino-IDF framework")