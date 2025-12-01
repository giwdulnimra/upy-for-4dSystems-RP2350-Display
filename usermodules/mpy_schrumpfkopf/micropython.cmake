# Create INERFACE-Library-Target
add_library(usermod_schrumpfkopf INTERFACE)

# add source files to the module
target_sources(usermod_schrumpfkopf INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/core.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/glue/mpy_glue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/micropython.wrapper.c
)

# include the current directory for header files
target_include_directories(usermod_schrumpfkopf INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${PICO_SDK_PATH}/src/rp2350/hardware_regs/include
)

target_link_libraries(usermod INTERFACE usermod_schrumpfkopf)
target_link_libraries(usermod_schrumpfkopf INTERFACE
        hardware_gpio
        hardware_pio
        pico_sdk_headers
        pico_stdlib
)
