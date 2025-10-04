
add_library(psram_tools INTERFACE)

target_sources(
    psram_tools
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/rp_pico_alloc.c
              ${CMAKE_CURRENT_LIST_DIR}/rp_pico_alloc.h
              ${CMAKE_CURRENT_LIST_DIR}/tlsf/tlsf.c
              ${CMAKE_CURRENT_LIST_DIR}/tlsf/tlsf.h
              ${CMAKE_CURRENT_LIST_DIR}/tlsf/tlsf_common.h
              ${CMAKE_CURRENT_LIST_DIR}/tlsf/tlsf_block_functions.h)

# Are we wrapping the built in alloc functions?
if (RP_PICO_ALLOC_WRAP)
    pico_wrap_function(psram_tools malloc)
    pico_wrap_function(psram_tools calloc)
    pico_wrap_function(psram_tools realloc)
    pico_wrap_function(psram_tools free)
endif ()

target_link_libraries(psram_tools INTERFACE pico_stdlib hardware_spi hardware_gpio hardware_flash pico_flash
                                              hardware_exception hardware_sync)
