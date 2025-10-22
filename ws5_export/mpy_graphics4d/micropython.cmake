# Create INERFACE-Library-Target
add_library(usermod_graphics4d INTERFACE)
set(GCX_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/src/4d.gcx")

# Configure output directrory
set(GFX_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
# Select Display
set(GFX_RGB_TYPE "70" CACHE STRING "RGB display type: 43,50,70,90")
# Füge Quellen hinzu (Bindings + Library)
target_sources(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modgraphics4d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Graphics4D.cpp
    # --- PSRAM Support ---
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/rp_pico_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/psram_tool.c
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/tlsf/tlsf.c
    # --- FatFS / SD Card Source Files ---
    # FatFs Core
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/ff15/source/diskio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/ff15/source/ff.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/ff15/source/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/ff15/source/ffunicode.c
    # SD Driver Core
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/dma_interrupts.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/sd_card.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/sd_timeouts.c
    # SPI Specific
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SPI/my_spi.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SPI/sd_card_spi.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SPI/sd_spi.c
    # SDIO Specific
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SDIO/rp2040_sdio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SDIO/sd_card_sdio.c
    # Utilities from .../src/src
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/src/f_util.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/src/my_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/src/rtc.c # May require pico_rtc dependency if used
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/src/util.c
    # --- Generated graphics object ---
    ${CMAKE_CURRENT_BINARY_DIR}/graphics.o
)

set(PICO_STDIO_USB 1)
set(PICO_STDIO_UART 0)
    
# Include-Pfade
target_include_directories(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/tlsf
    ${CMAKE_CURRENT_LIST_DIR}/src/fonts
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/ff15/source
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/include
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SPI
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SDIO
    ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

# generate_graphics
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx
    COMMAND ${CMAKE_COMMAND} -E copy ${GCX_SOURCE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx
    DEPENDS ${GCX_SOURCE_PATH} COMMENT "Copying 4d.gcx to ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx"
) 
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/graphics.o
    COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-littlearm -B arm --rename-section .data=.graphics 4d.gcx ${CMAKE_CURRENT_BINARY_DIR}/graphics.o
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Converting graphics file to ${CMAKE_CURRENT_BINARY_DIR}/graphics.o"
)
add_custom_target(generate_graphics ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/graphics.o)

# select PIO-File
if(GFX_RGB_TYPE STREQUAL "43")
    set(GFX_PIO_FILE rgb43.pio)
elseif(GFX_RGB_TYPE STREQUAL "50")
    set(GFX_PIO_FILE rgb50.pio)
elseif(GFX_RGB_TYPE STREQUAL "70")
    set(GFX_PIO_FILE rgb70.pio)
elseif(GFX_RGB_TYPE STREQUAL "90")
    set(GFX_PIO_FILE rgb90.pio)
elseif(GFX_RGB_TYPE STREQUAL "2040")
    set(GFX_PIO_FILE bus_2040.pio)
else()
    message(FATAL_ERROR "Unknown GFX_RGB_TYPE: ${GFX_RGB_TYPE}")
endif()
set(GFX_PIO_FULL ${CMAKE_CURRENT_LIST_DIR}/src/${GFX_PIO_FILE})
set(GFX_PIO_HEADER ${GFX_OUT_DIR}/${GFX_PIO_FILE}.h)
# Create PIO Header
pico_generate_pio_header(usermod_graphics4d ${GFX_PIO_FULL} OUTPUT_DIR ${GFX_OUT_DIR})
set_source_files_properties(${GFX_PIO_HEADER} PROPERTIES GENERATED TRUE)

set(SDIO_PIO_FILE rp2040_sdio.pio)
set(SDIO_PIO_FULL ${CMAKE_CURRENT_LIST_DIR}/src/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SDIO/${SDIO_PIO_FILE})
set(SDIO_PIO_HEADER ${GFX_OUT_DIR}/${SDIO_PIO_FILE}.h)
pico_generate_pio_header(usermod_graphics4d ${SDIO_PIO_FULL} OUTPUT_DIR ${GFX_OUT_DIR})
set_source_files_properties(${SDIO_PIO_HEADER} PROPERTIES GENERATED TRUE)

# Ensure Timing on frozen/qstr-Generierung
add_dependencies(usermod generate_graphics)
add_dependencies(usermod picotool)

# add PIO Headers to sources
target_sources(usermod_graphics4d INTERFACE
    ${GFX_PIO_HEADER}
    ${SDIO_PIO_HEADER}
)

target_compile_definitions(usermod_graphics4d INTERFACE
    USE_4D_FONT1
    USE_4D_FONT2
    USE_4D_FONT3
    USE_4D_FONT4
)

# Verknüpfe mit dem generischen usermod-Target (MicroPython-Link)
target_link_libraries(usermod INTERFACE usermod_graphics4d)
target_link_libraries(usermod_graphics4d INTERFACE pico_stdio_usb)