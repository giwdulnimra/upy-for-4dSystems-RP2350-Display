# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     =
    "10.19.28.28"
    #"code.protronic.local"
    #"can-mon"
$LocalBaseDir = "."
$shContent = @"
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

cd ~/micropython/lib/
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init

cd ~/micropython
make -j$(nproc) -C mpy-cross
"@

Set-Content -Path "$LocalBaseDir/config/config_build.sh" -Value $shContent  -Encoding UTF8
scp .\config\config_build.sh "${RemoteUser}@${RemoteHost}:~/config_build.sh"
ssh $RemoteUser@$RemoteHost "bash ~/config_build.sh"