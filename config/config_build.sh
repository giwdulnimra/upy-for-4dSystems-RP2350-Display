#!/bin/bash

if [ -d "C:\Users\armin/micropython" ]; then
    git fetch
    git checkout dev-branch || git checkout -b dev-branch origin/main
    git branch --set-upstream-to=origin/main dev-branch
    git pull
else
    git clone https://github.com/giwdulnimra/micropython.git
    cd micropython
    git checkout -b dev-branch
fi

sudo apt-get update
sudo apt-get install -y build-essential libffi-dev git pkg-config gcc-arm-none-eabi libnewlib-arm-none-eabi cmake libssl-dev

cd ~/micropython/lib/
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init

cd ~/micropython/tools
git clone https://github.com/raspberrypi/picotool.git
cd ~/micropython/tools/picotool/
mkdir -p build
cd build
cmake -DPICO_SDK_PATH=/home/armin/micropython/lib/pico-sdk
make -j4
export PICOTOOL=/home/armin/micropython/tools/picotool/build/picotool

cd ~/micropython
make -j4 -C mpy-cross
