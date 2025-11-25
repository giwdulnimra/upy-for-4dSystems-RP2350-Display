set(MODULE_NAME core)
set(USER_C_MODULES ${USER_C_MODULES} ${USERMOD_DIR})

set(cpp_hardware_SRCS_C
        ${USERMOD_DIR}/core.cpp
)

set(cpp_hardware_SRCS_CPP
        ${USERMOD_DIR}/core.cpp
)

set(cpp_hardware_INC
        ${USERMOD_DIR}
        ${USERMOD_DIR}/src
)
set(${MODULE_NAME}_INC
        ${USERMOD_DIR}
        ${USERMOD_DIR}/src
)
target_link_libraries(cpp_hardware INTERFACE pico_stdlib)
target_link_libraries(${MODULE_NAME} INTERFACE pico_stdlib)


# CMake configuration for the hello world user module
#add_library(usermod_hello_world INTERFACE)

# add source files to the module
#target_sources(usermod_hello_world INTERFACE
#    ${CMAKE_CURRENT_LIST_DIR}/src/helloworld.c
#)

# include the current directory for header files
#target_include_directories(usermod_hello_world INTERFACE
#    ${CMAKE_CURRENT_LIST_DIR}
#    ${CMAKE_CURRENT_LIST_DIR}/src
#)

# Link the user module to the main usermod target
#target_link_libraries(usermod INTERFACE usermod_hello_world)
