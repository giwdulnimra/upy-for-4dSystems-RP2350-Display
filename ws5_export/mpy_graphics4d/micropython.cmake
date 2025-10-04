# Erstelle ein INERFACE-Library-Target
add_library(usermod_graphics4d INTERFACE)

# Füge Quellen hinzu (Bindings + Library)
target_sources(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modgraphics4d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lib/Graphics4D.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lib/psram_tools/rp_pico_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/psram_tools/psram_tool.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/psram_tools/tlsf/tlsf.c
)

# Include-Pfade
target_include_directories(usermod_graphics4d INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lib
    ${CMAKE_CURRENT_LIST_DIR}/lib/psram_tools
    ${CMAKE_CURRENT_LIST_DIR}/lib/psram_tools/tlsf
)

# Verknüpfe mit dem generischen usermod-Target
target_link_libraries(usermod INTERFACE usermod_graphics4d)

pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/lib/rgb43.pio)
pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/lib/rgb50.pio)
pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/lib/rgb70.pio)
pico_generate_pio_header(graphics_4d ${CMAKE_CURRENT_LIST_DIR}/lib/rgb90.pio)