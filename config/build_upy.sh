#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2
export BOARD=RPI_PICO2_W
export BUILD=build-Pico2w
#export USER_C_MODULES=
make -j$(nproc) submodules

while true; do
    echo "Run 'make clean'? [y/n]: "
    read run_clean
    case $run_clean in
        [yY])
            make -j$(nproc) clean
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
make -j$(nproc)
