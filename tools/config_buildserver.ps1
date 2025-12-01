# ---------------- CONFIG -----------------
$config = Get-Content "config.json" | ConvertFrom-Json
$RemoteUser     = $config.user
$RemoteHost     = $config.server
$LocalBaseDir = "."
$shContent = @"
#!/bin/bash
echo "-- Starting setup (ScriptV8) --"
cd ~
if [ -d "micropython" ]; then
    echo "Updating MicroPython repository..."
    cd ~/micropython
    git rebase --abort 2>/dev/null || true
    git fetch
    git checkout dev-branch || git checkout -b dev-branch origin/master
    git branch --set-upstream-to=origin/master dev-branch
    git pull --rebase
else
    echo "Cloning MicroPython repository..."
    git clone https://github.com/giwdulnimra/micropython.git
    cd micropython
    git checkout -b dev-branch
fi
echo "Updating submodules..."
git submodule update --init --recursive
sudo apt-get update
sudo apt-get install -y build-essential libffi-dev git pkg-config gcc-arm-none-eabi libnewlib-arm-none-eabi cmake libssl-dev

cd ~/micropython/mpy-cross/
make -j$(nproc)

"@

Set-Content -Path "$LocalBaseDir/config_build.sh" -Value ($shContent -replace "`r`n", "`n") -Encoding UTF8 -NoNewline
scp .\config_build.sh "${RemoteUser}@${RemoteHost}:~/config_build.sh"
ssh $RemoteUser@$RemoteHost "bash ~/config_build.sh"
