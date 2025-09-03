#!/usr/bin/env python3
"""
ESP32-P4 Upload Script for Native ESP-IDF Projects
This script properly handles ESP32-P4 chip detection and flashing
"""

import subprocess
import sys
import os
from pathlib import Path

def main():
    # Get environment variables
    project_dir = os.environ.get('PROJECT_DIR', '.')
    pioenv = os.environ.get('PIOENV', 'esp32p4_native')
    upload_port = os.environ.get('UPLOAD_PORT', '/dev/ttyACM1')
    upload_speed = os.environ.get('UPLOAD_SPEED', '460800')
    
    # Build directory
    build_dir = Path(project_dir) / '.pio' / 'build' / pioenv
    
    # Required binary files for ESP32-P4
    bootloader_bin = build_dir / 'bootloader.bin'
    partitions_bin = build_dir / 'partitions.bin' 
    firmware_bin = build_dir / 'firmware.bin'
    
    # Check if files exist
    missing_files = [f for f in [bootloader_bin, partitions_bin, firmware_bin] if not f.exists()]
    if missing_files:
        print(f"Error: Missing binary files: {missing_files}")
        print("Run 'pio run' to build the project first.")
        return 1
    
    # ESP32-P4 flash command with proper addresses
    cmd = [
        'esptool.py', '--chip', 'esp32p4',
        '--port', upload_port, '--baud', upload_speed,
        'write_flash',
        '--flash_mode', 'qio',
        '--flash_freq', '80m', 
        '--flash_size', '16MB',
        '--force',
        # Bootloader at 0x0000
        '0x0000', str(bootloader_bin),
        # Partition table at 0x8000  
        '0x8000', str(partitions_bin),
        # Application at 0x10000
        '0x10000', str(firmware_bin)
    ]
    
    print("=" * 60)
    print("ESP32-P4 Native ESP-IDF Upload")
    print("=" * 60)
    print(f"Port: {upload_port}")
    print(f"Baud: {upload_speed}")
    print(f"Bootloader: {bootloader_bin}")
    print(f"Partitions: {partitions_bin}")
    print(f"Firmware: {firmware_bin}")
    print("=" * 60)
    
    # Execute upload
    try:
        result = subprocess.run(cmd, check=True)
        print("Upload completed successfully!")
        return 0
    except subprocess.CalledProcessError as e:
        print(f"Upload failed with error code {e.returncode}")
        return e.returncode
    except Exception as e:
        print(f"Upload failed: {e}")
        return 1

if __name__ == '__main__':
    sys.exit(main())