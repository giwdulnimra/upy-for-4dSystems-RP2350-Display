##v01
# Makefile for including Graphics4D in MicroPython

# This assumes your Graphics4D source files and the wrapper are in a directory
# that the build system can find. For example, in the same directory as this makefile.

# Add the source files for your module to the build.
SRC_USERMOD += graphics4d_wrapper.cpp
SRC_USERMOD += Graphics4D.cpp 

# Add compiler flags for C++.
CXXFLAGS += -std=c++11

# You may need to add include paths for your library's headers
CFLAGS += -I$(USERMOD_DIR)



##v02
# Makefile for including the native Graphics4D module in MicroPython

# Add the C++ source files for your module to the build.
# The build system must be configured to compile C++ files.
SRC_USERMOD_CXX += $(USERMOD_DIR)/graphics4d_micropython_wrapper.cpp
SRC_USERMOD_CXX += $(USERMOD_DIR)/Graphics4D.cpp

# Add the directory of this module to the C include paths.
CFLAGS_USERMOD += -I$(USERMOD_DIR)
