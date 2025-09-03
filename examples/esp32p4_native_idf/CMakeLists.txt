cmake_minimum_required(VERSION 3.16)

# Set the target before calling project()
set(IDF_TARGET esp32p4)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32p4_native)