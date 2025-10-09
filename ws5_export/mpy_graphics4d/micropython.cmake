# Erstelle ein INERFACE-Library-Target
add_library(usermod_graphics4d INTERFACE)

# Füge Quellen hinzu (Bindings + Library)
target_sources(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modgraphics4d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Graphics4D.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/rp_pico_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/psram_tool.c
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/tlsf/tlsf.c
)

# Include-Pfade
target_include_directories(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools
    ${CMAKE_CURRENT_LIST_DIR}/src/psram_tools/tlsf
    ${CMAKE_CURRENT_BINARY_DIR}/src
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
    USE_4D_FONT2
    USE_4D_FONT3
    USE_4D_FONT4
)

# Verknüpfe mit dem generischen usermod-Target (MicroPython-Link)
target_link_libraries(usermod INTERFACE usermod_graphics4d)