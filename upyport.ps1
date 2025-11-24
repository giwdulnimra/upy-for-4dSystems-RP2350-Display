# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     = "10.19.28.130"
$LocalBaseDir   = "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
$TargetBoard    = "4DSYS_RP2350_70"
                  #"RPI_PICO2_W"
$BuildDirectory = "build-4Dsys"
                  #"build-Pico2w"
$ClearBuild     = #$false
                  $true
# -----------------------------------------
# ------- COPY WS5-PROJEKT TO HOST --------
Clear-Host
scp -r "$LocalBaseDir/custom_ports/rp2/boards/$TargetBoard/" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/boards/"
scp "$LocalBaseDir/config/extmod.cmake" "${RemoteUser}@${RemoteHost}:~/micropython/extmod/extmod.cmake"
scp "$LocalBaseDir/config/CMakeLists.txt" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/CMakeLists.txt"

# ---------------- BUILD ------------------
$shContent = @"
#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2

make -j4 V=1 submodules BOARD=$TargetBoard BUILD=$BuildDirectory

while true; do
    echo "Run 'make clean'? [y/n]: "
    read run_clean
    case `$run_clean in
        [yY])
            make -j4 V=1 clean BUILD=$BuildDirectory
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
make -j4 BOARD=$TargetBoard BUILD=$BuildDirectory


"@

Set-Content -Path "$LocalBaseDir/config/build_upy.sh" -Value ($shContent -replace "`r`n", "`n") -Encoding UTF8 -NoNewline
scp .\config\build_upy.sh "${RemoteUser}@${RemoteHost}:~/build_upy.sh"
ssh $RemoteUser@$RemoteHost "bash ~/build_upy.sh"

# --------- COPY BUILD FROM HOST ----------
#scp -r "${RemoteUser}@${RemoteHost}/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}" "$LocalBaseDir/build/"

New-Item -ItemType Directory -Path "$LocalBaseDir/$BuildDirectory/frozen_mpy" -Force
New-Item -ItemType Directory -Path "$LocalBaseDir/$BuildDirectory/genhdr" -Force
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.uf2" "${LocalBaseDir}/build/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.bin" "${LocalBaseDir}/build/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.hex" "${LocalBaseDir}/build/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.dis" "${LocalBaseDir}/build/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.elf" "${LocalBaseDir}/build/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.elf.map" "${LocalBaseDir}/build/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/*.c" "${LocalBaseDir}/build/${BuildDirectory}/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/frozen_mpy/*.mpy" "${LocalBaseDir}/build/${BuildDirectory}/frozen_mpy/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}/genhdr/*.h" "${LocalBaseDir}/build/${BuildDirectory}/genhdr/"
if ($ClearBuild) {
    Write-Host "removing remote build directory..."   
    ssh $RemoteUser@$RemoteHost "rm -r /home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}"
}