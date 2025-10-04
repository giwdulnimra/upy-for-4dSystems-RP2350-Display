# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     = 
    "10.19.28.28"
    #"code.protronic.local"
    #"can-mon"
$LocalBaseDir = #"."
    "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
$TargetBoard = "4DSYS_RP2350_70"
    #"RPI_PICO2_W"
$BuildDirectory = "build-4Dsys"
    #"build-Pico2w"
    #"build-CustomTest"
# -----------------------------------------
# ------- COPY WS5-PROJEKT TO HOST --------
scp -r $LocalBaseDir"/ws5_export" "${RemoteUser}@${RemoteHost}:~/micropython/"
scp -r $LocalBaseDir"/rp2/boards/$TargetBoard/" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/boards/"

# ---------------- BUILD ------------------
$shContent = @"
#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2
export BOARD=$TargetBoard
export BUILD=$BuildDirectory
#export USER_C_MODULES=~/micropython/ws5_export/Graphics4D/src

make -j4 submodules
"@+@'

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


'@

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
