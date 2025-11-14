#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2

make -j4 V=1 submodules BOARD=4DSYS_RP2350_70 BUILD=build-4Dsys USER_C_MODULES=~/micropython/ws5_export/mpy_graphics4d/micropython.cmake

while true; do
    echo "Run 'make clean'? [y/n]: "
    read run_clean
    case $run_clean in
        [yY])
            make -j4 V=1 clean
            break
            ;;        
        [nN])
            echo "Skipping 'make clean'"
            break
            ;;
        *)
            ;;
    esac
done
make -j4 BOARD=4DSYS_RP2350_70 BUILD=build-4Dsys USER_C_MODULES=~/micropython/ws5_export/mpy_graphics4d/micropython.cmake

