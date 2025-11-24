cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(${WS5_PROJECT_NAME}_${PICO_BOARD})

add_compile_definitions(
    USE_4D_FONT2
    USE_4D_FONT3
    USE_4D_FONT4
)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(Graphics4D)

# Copy the generated graphics file to the build directory
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/4d.gcx
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/${WS5_PROJECT_NAME}.gcx ${CMAKE_BINARY_DIR}/4d.gcx
    DEPENDS ${CMAKE_SOURCE_DIR}/${WS5_PROJECT_NAME}.gcx
    COMMENT "Copying ${WS5_PROJECT_NAME}.gcx to ${CMAKE_BINARY_DIR}/4d.gcx"
)

# Convert the copied graphics file into an object file using objcopy
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/graphics.o
    COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-littlearm -B arm --rename-section .data=.graphics 4d.gcx ${CMAKE_BINARY_DIR}/graphics.o
    DEPENDS ${CMAKE_BINARY_DIR}/4d.gcx
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Converting graphics file to ${CMAKE_BINARY_DIR}/graphics.o"
)

# Create a custom target that will generate graphics.o during build
add_custom_target(generate_graphics ALL
    DEPENDS ${CMAKE_BINARY_DIR}/graphics.o
)

# Create the executable
add_executable(${PROJECT_NAME}
    main.cpp
    ${CMAKE_BINARY_DIR}/graphics.o
)

set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")


# Link the object file that contains the embedded binary data
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_pwm hardware_pio hardware_flash pico_flash hardware_i2c hardware_clocks psram_tools pico_bootrom graphics_4d)

# Specify the linker script
pico_set_linker_script(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/Graphics4D/extras/memmap_psram_8mb.ld)

# Enable stdio
pico_enable_stdio_usb(${PROJECT_NAME} 1) # allows programming with Workshop5 without manually resetting to bootloader mode
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})