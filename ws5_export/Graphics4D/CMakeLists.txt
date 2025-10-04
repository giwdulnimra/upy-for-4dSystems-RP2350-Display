cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

# initialize the SDK based on PICO_SDK_PATH
# this assumes that it is used in a Pico SDK project and the 
# specified file is just outside of the library folder
include(../pico_sdk_import.cmake)

add_subdirectory(src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src)
add_subdirectory(src/psram_tools)

add_library(graphics_4d
    src/Graphics4D.cpp
)

pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/src/rgb43.pio)
pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/src/rgb50.pio)
pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/src/rgb70.pio)
pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/src/rgb90.pio)

target_link_libraries(graphics_4d pico_stdlib no-OS-FatFS-SD-SDIO-SPI-RPi-Pico hardware_clocks hardware_dma hardware_flash hardware_pwm hardware_adc hardware_pio hardware_i2c pico_flash pico_bootrom pico_stdio_usb psram_tools)

target_include_directories(graphics_4d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
