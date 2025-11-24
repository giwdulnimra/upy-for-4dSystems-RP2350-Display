# CMake configuration for the hello world user module
add_library(usermod_hello_world INTERFACE)

# add source files to the module
target_sources(usermod_hello_world INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/helloworld.c
)

# include the current directory for header files
target_include_directories(usermod_hello_world INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link the user module to the main usermod target
target_link_libraries(usermod INTERFACE usermod_hello_world)