# PATH config
GRAPHICS4D_DIR := $(USERMOD_DIR)

# original C++-Library
SRC_USERMOD_LIB_CXX += \
	$(GRAPHICS4D_DIR)/lib/Graphics4D.cpp
# Binding/Wrapper-File
SRC_USERMOD_CXX += \
	$(GRAPHICS4D_DIR)/modgraphics4d.cpp

# C-Sources PSRAM-Support
SRC_USERMOD_C += \
	$(GRAPHICS4D_DIR)/src/psram_tools/rp_pico_alloc.c \
	$(GRAPHICS4D_DIR)/src/psram_tools/psram_tool.c \
	$(GRAPHICS4D_DIR)/src/psram_tools/tlsf/tlsf.c

# Assembly-Sources zur automatischen pioasm in rp2-Make
SRC_USERMOD_LIB_ASM += \
	$(GRAPHICS4D_DIR)/src/rgb43.pio \
	$(GRAPHICS4D_DIR)/src/rgb50.pio \
	$(GRAPHICS4D_DIR)/src/rgb70.pio \
	$(GRAPHICS4D_DIR)/src/rgb90.pio \
	$(GRAPHICS4D_DIR)/src/bus_2040.pio

# Falls zusätzliche Include-Pfade nötig sind:
CXXFLAGS_USERMOD += \
	-I$(GRAPHICS4D_DIR)/ \
	-I$(GRAPHICS4D_DIR)/src/ \
	-I$(GRAPHICS4D_DIR)/src/psram_tools/ \
	-I$(GRAPHICS4D_DIR)/src/psram_tools/tlsf/
	-I$(GRAPHICS4D_DIR)/src/fonts/

# Fonts aktivieren
CXXFLAGS_USERMOD += \
	-DUSE_4D_FONT1 \
	-DUSE_4D_FONT2 \
	-DUSE_4D_FONT3 \
	-DUSE_4D_FONT4
