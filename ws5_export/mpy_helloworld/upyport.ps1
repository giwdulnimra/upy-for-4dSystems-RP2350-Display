# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     = "10.19.28.130"
$LocalBaseDir   = "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
$TargetBoard    = "4DSYS_RP2350_70"
                #"RPI_PICO2_W"
$BuildDirectory = "build-Test"
$ClearBuild     = #$false
                $true
# -----------------------------------------
# ------- COPY WS5-PROJEKT TO HOST --------
Clear-Host
scp -r "$LocalBaseDir/ws5_export/mpy_helloworld/" "${RemoteUser}@${RemoteHost}:~/micropython/ws5_export/"

# ---------------- BUILD ------------------
$shContent = @"
#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2

make -j4 V=1 submodules BOARD=$TargetBoard BUILD=$BuildDirectory USER_C_MODULES=~/micropython/ws5_export/mpy_helloworld/micropython.cmake
make -j4 V=1 clean BUILD=$BuildDirectory
make -j4 BOARD=$TargetBoard BUILD=$BuildDirectory USER_C_MODULES=~/micropython/ws5_export/mpy_helloworld/micropython.cmake


"@

Set-Content -Path "$LocalBaseDir/ws5_export/mpy_helloworld/config/build_upy.sh" -Value ($shContent -replace "`r`n", "`n") -Encoding UTF8 -NoNewline
scp "$LocalBaseDir/ws5_export/mpy_helloworld/config/build_upy.sh" "${RemoteUser}@${RemoteHost}:~/tests/build_upy.sh"
ssh $RemoteUser@$RemoteHost "bash ~/tests/build_upy.sh"

# --------- COPY BUILD FROM HOST ----------
scp -r "${RemoteUser}@${RemoteHost}/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}" $LocalBaseDir
#mkdir -p "$LocalBaseDir/$BuildDirectory/frozen_mpy"
#mkdir -p "$LocalBaseDir/$BuildDirectory/genhdr"
New-Item -ItemType Directory -Path "$LocalBaseDir/$BuildDirectory/frozen_mpy" -Force
New-Item -ItemType Directory -Path "$LocalBaseDir/$BuildDirectory/genhdr" -Force
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.uf2" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.bin" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.hex" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.dis" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.elf" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.elf.map" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.c" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/frozen_mpy/*.mpy" "${LocalBaseDir}/${BuildDirectory}/frozen_mpy/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/genhdr/*.h" "${LocalBaseDir}/${BuildDirectory}/genhdr/"
if ($ClearBuild) {
    Write-Host "removing remote build directory..."   
    ssh $RemoteUser@$RemoteHost "rm -r /home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}"
}