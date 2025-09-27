#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2
export BOARD=4DSYS_RP2350_70
export BUILD=build-4Dsys
#export USER_C_MODULES=

make -j4 submodules
while true; do
    echo "Run 'make clean'? [y/n]: "
    read run_clean
    case $run_clean in
        [yY])
            make -j4 clean
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
make -j4


