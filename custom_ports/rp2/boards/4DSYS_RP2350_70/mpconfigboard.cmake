# The Core is powered by an RP2350B with 48 GPIOs
set(PICO_NUM_GPIOS 48)

# Not all gen4 boards have official pico-sdk support. Check ~/micropython/lib/pico-sdk/src/boards/include/ for your board.
# If nessescary copy board file from Workshop 5 ibstallation, add this board directory to the header search path and define PICO_BOARD
# Check C:/Program Files/4D Labs/4D Workshop 5 IDE/rp2350/boards/ or similar
# This will instruct pico-sdk to look for gen4_rp2350_70ct.h
list(APPEND PICO_BOARD_HEADER_DIRS ${MICROPY_BOARD_DIR})
set(PICO_BOARD "gen4_rp2350_70ct")

# Freeze manifest and modules
set(MICROPY_FROZEN_MANIFEST ${MICROPY_BOARD_DIR}/manifest.py)
