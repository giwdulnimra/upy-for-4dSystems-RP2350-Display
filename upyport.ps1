# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     = 
    "10.19.28.28"
    #"code.protronic.local"
    #"can-mon"
$LocalBaseDir = #"."
    "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
$TargetBoard = "RPI_PICO2_W" #"4DSYS_RP2350"
$BuildDirectory = "build-Pico2w" #"build-4Dsys"
    #"build-CustomTest"
# -----------------------------------------
# ------- COPY WS5-PROJEKT TO HOST --------
#scp -rv $LocalBaseDir"/ws5_export" "${RemoteUser}@${RemoteHost}:~/micropython/"
#scp -rv $LocalBaseDir"/micropython/ports/rp2/boards/4DSYS_RP2350/" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/boards/"

# ---------------- BUILD ------------------
$shContent = @"
#!/bin/bash
cd ~/micropython
#make -C mpy-cross -j$(nproc)
cd ports/rp2
export BOARD=$TargetBoard
export BUILD=$BuildDirectory
#export USER_C_MODULES=
#make -j$(nproc) BOARD=$TargetBoard BUILD=$BuildDirectory submodules
#make -j$(nproc) BOARD=$TargetBoard BUILD=$BuildDirectory clean
#make -j$(nproc) BOARD=$TargetBoard BUILD=$BuildDirectory 
make -j$(nproc) submodules
#make -j$(nproc) clean
make -j$(nproc)
#FIRMWARE=4dsys_upy.uf2
"@
Set-Content -Path "$LocalBaseDir/config/build_upy.sh" -Value $shContent  -Encoding UTF8
scp .\config\build_upy.sh "${RemoteUser}@${RemoteHost}:~/build_upy.sh"
ssh $RemoteUser@$RemoteHost "bash ~/build_upy.sh"

# --------- COPY BUILD FROM HOST ----------
#scp -r "${RemoteUser}@${RemoteHost}/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}" $LocalBaseDir
mkdir -p "$LocalBaseDir/$BuildDirectory/frozen_mpy"
mkdir -p "$LocalBaseDir/$BuildDirectory/genhdr"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.uf2" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.bin" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.hex" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.dis" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.elf" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.elf.map" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.c" "${LocalBaseDir}/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/frozen_mpy/*.mpy" "${LocalBaseDir}/${BuildDirectory}/frozen_mpy/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/genhdr/*.h" "${LocalBaseDir}/${BuildDirectory}/genhdr/"
