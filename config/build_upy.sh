#!/bin/bash
cd ~/micropython
#make -C mpy-cross -j4
cd ports/rp2
export BOARD=RPI_PICO2_W
export BUILD=build-Pico2w
#export USER_C_MODULES=
#make -j4 BOARD=RPI_PICO2_W BUILD=build-Pico2w submodules
#make -j4 BOARD=RPI_PICO2_W BUILD=build-Pico2w clean
#make -j4 BOARD=RPI_PICO2_W BUILD=build-Pico2w 
make -j4 submodules
#make -j4 clean
make -j4
#FIRMWARE=4dsys_upy.uf2
