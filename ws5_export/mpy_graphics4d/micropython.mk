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
	-DUSE_4D_FONT4 \
	-I$GRAPHICS4D_DIR/src

# Rule to convert the raw binary graphics data (4d.gcx) into a linkable object file (graphics.o)
$(GCX_OBJ_FILE): $(GCX_SOURCE)
	@echo "GEN $(notdir $@) from $(notdir $<)"
	@mkdir -p $(BUILD)
	$(Q)cp $< $(GCX_BIN_FILE)
	$(Q)$(OBJCOPY) -I binary -O elf32-littlearm -B arm --rename-section .data=.graphics $(GCX_BIN_FILE) $@

# 4D-GCX-File
GCX_SOURCE := \
	$(GRAPHICS4D_DIR)/src/4d.gcx
GCX_BIN_FILE := $(BUILD)/$(notdir $(GCX_SOURCE))
GCX_OBJ_FILE := $(BUILD)/graphics.o
SRC_USERMOD_OBJS += $(GCX_OBJ_FILE)
