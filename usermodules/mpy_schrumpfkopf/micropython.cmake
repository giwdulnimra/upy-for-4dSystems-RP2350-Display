set(USER_C_MODULES ${USER_C_MODULES} ${USERMOD_DIR})

set(cpp_hardware_SRCS_C
        ${USERMOD_DIR}/module.cpp
)

set(cpp_hardware_SRCS_CPP
        ${USERMOD_DIR}/led_driver.cpp
)

set(cpp_hardware_INC
        ${USERMOD_DIR}
)

target_link_libraries(cpp_hardware INTERFACE pico_stdlib)