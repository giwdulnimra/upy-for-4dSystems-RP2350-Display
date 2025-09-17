#!/bin/bash

if [ -d "$HOME/micropython" ]; then
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
make -C mpy-cross
