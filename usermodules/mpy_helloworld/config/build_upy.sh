#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2

make -j4 V=1 submodules BOARD=4DSYS_RP2350_70 BUILD=build-Test USER_C_MODULES=~/micropython/ws5_export/mpy_helloworld/micropython.cmake
make -j4 V=1 clean BUILD=build-Test
make -j4 BOARD=4DSYS_RP2350_70 BUILD=build-Test USER_C_MODULES=~/micropython/ws5_export/mpy_helloworld/micropython.cmake

