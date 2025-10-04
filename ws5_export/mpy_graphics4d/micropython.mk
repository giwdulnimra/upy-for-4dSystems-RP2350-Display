# PATH config
GRAPHICS4D_DIR := $(USERMOD_DIR)

# original C++-Library
SRC_USERMOD_LIB_CXX += $(GRAPHICS4D_DIR9)/lib/Graphics4D.cpp

# C-Sources PSRAM-Support
SRC_USERMOD_C += \
	$(GRAPHICS4D_DIR)/lib/psram_tools/rp_pico_alloc.c \
	$(GRAPHICS4D_DIR)/lib/psram_tools/psram_tool.c \
	$(GRAPHICS4D_DIR)/lib/psram_tools/tlsf/tlsf.c

# Binding/Wrapper-File
SRC_USERMOD_CXX += $(GRAPHICS4D_DIR)/modgraphics4d.cpp

# Falls zusätzliche Iclude-Pfade nötig sind:
CXXFLAGS_USERMOD += \
	-I$(GRAPHICS4D_DIR)/ \
	-I$(GRAPHICS4D_DIR)/lib/ \
	-I$(GRAPHICS4D_DIR)/lib/psram_tools/ \
	-I$(GRAPHICS4D_DIR)/lib/psram_tools/tlsf/

# --
# SRC_USERMOD_C += $(GRAPHICS4D_DIR)/modexample.c
# SRC_USERMOD_LIB_C += $(GRAPHICS4D_DIR)/$(LIBPATH1)/algorithm.c
# SRC_USERMOD_CXX += $(GRAPHICS4D_DIR)/modexample.cpp
# SRC_USERMOD_LIB_CXX += $(GRAPHICS4D_DIR)/$(LIBPATH1)/algorithm.cpp