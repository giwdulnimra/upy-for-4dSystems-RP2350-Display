# Erstelle ein INERFACE-Library-Target
add_library(usermod_graphics4d INTERFACE)
set(GCX_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/src/4d.gcx")

# Füge Quellen hinzu (Bindings + Library)
target_sources(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modgraphics4d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Graphics4D.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/rp_pico_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/psram_tool.c
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/tlsf/tlsf.c
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
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx
    COMMAND ${CMAKE_COMMAND} -E copy ${GCX_SOURCE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx
    DEPENDS ${GCX_SOURCE_PATH} 
    COMMENT "Copying 4d.gcx to ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx"
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/graphics.o
    COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-littlearm -B arm --rename-section .data=.graphics 4d.gcx ${CMAKE_CURRENT_BINARY_DIR}/graphics.o
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/4d.gcx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Converting graphics file to ${CMAKE_CURRENT_BINARY_DIR}/graphics.o"
)
add_custom_target(generate_graphics ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/graphics.o
)

pico_generate_pio_header(usermod_graphics4d 
    INPUT ${CMAKE_CURRENT_LIST_DIR}/src/rgb43.pio
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    )
pico_generate_pio_header(usermod_graphics4d 
    INPUT ${CMAKE_CURRENT_LIST_DIR}/src/rgb50.pio
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    )
pico_generate_pio_header(usermod_graphics4d
    INPUT ${CMAKE_CURRENT_LIST_DIR}/src/rgb70.pio
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    )
pico_generate_pio_header(usermod_graphics4d
    INPUT ${CMAKE_CURRENT_LIST_DIR}/src/rgb90.pio
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    )
pico_generate_pio_header(usermod_graphics4d
    INPUT ${CMAKE_CURRENT_LIST_DIR}/src/bus_2040.pio
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    )

# PIO-Header as Sources
target_sources(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/src/rgb43.pio.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/rgb50.pio.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/rgb70.pio.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/rgb90.pio.h
)

target_compile_definitions(usermod_graphics4d INTERFACE
    USE_4D_FONT1
    USE_4D_FONT2
    USE_4D_FONT3
    USE_4D_FONT4
)

# Verknüpfe mit dem generischen usermod-Target (MicroPython-Link)
target_link_libraries(usermod INTERFACE usermod_graphics4d)