# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     = "10.19.28.130"
$LocalBaseDir   = "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
                  #"./../../"
$TargetBoard    = "4DSYS_RP2350_70"
                  #"RPI_PICO2_W"
$BuildDirectory = "build-4Dsys"
$ClearBuild     = #$false
                  $true
$ProjektName    = "mpy_graphics4d"
$Main           = "SliderExample"
# -----------------------------------------
# ------- COPY WS5-PROJEKT TO HOST --------
Clear-Host
Copy-Item $LocalBaseDir"/usermodules/$ProjektName/ws5_export/$Main.gcx" $LocalBaseDir"/usermodules/$ProjektName/src/4d.gcx" -Force
Copy-Item $LocalBaseDir"/usermodules/$ProjektName/ws5_export/GeneratedConsts.h" $LocalBaseDir"/usermodules/$ProjektName/src/GeneratedConsts.h" -Force
#Copy-Item $LocalBaseDir"/usermodules/$ProjektName/ws5_export/Graphics4D/src/Graphics4D.cpp" $LocalBaseDir"/usermodules/$ProjektName/Graphics4D_OriginCopy.cpp" -Force
#Copy-Item $LocalBaseDir"/usermodules/$ProjektName/ws5_export/Graphics4D/src/Graphics4D.h" $LocalBaseDir"/usermodules/$ProjektName/Graphics4D_OriginCopy.h" -Force
$selected = $false
while (-not $selected) {
    Write-Host "The Build Folder contains files that should not be changed between builds (sources). Copying these files can take some time but is necessary at least once. Type 'a' to copy the entire build folder, 'e' to copy just essential build files, or 'b' to just build without copying: "
    $build_choice = Read-Host
    switch ($build_choice.ToLower()) {
        'a' {
            scp -r "$LocalBaseDir/usermodules/$ProjectName" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName"
            scp -r "$LocalBaseDir/custom_ports/rp2/boards/$TargetBoard/" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/boards/"
            scp "$LocalBaseDir/usermodules/$ProjectName/config/extmod.cmake" "${RemoteUser}@${RemoteHost}:~/micropython/extmod/extmod.cmake"
            scp "$LocalBaseDir/usermodules/$ProjectName/config/CMakeLists.txt" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/CMakeLists.txt"
            $selected = $true
            break
        }
        'e' {
            scp "$LocalBaseDir/usermodules/$ProjectName/modgraphics4d.cpp" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/*.c"
            scp "$LocalBaseDir/usermodules/$ProjectName/modgraphics4d.cpp" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/*.cpp"
            #scp "$LocalBaseDir/usermodules/$ProjectName/modgraphics4d.h" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/modgraphics4d.cpp"
            scp "$LocalBaseDir/usermodules/$ProjectName/micropython.cmake" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/micropython.cmake"
            #scp "$LocalBaseDir/usermodules/$ProjectName/micropython.mk" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/micropython.mk"
            scp "$LocalBaseDir/usermodules/$ProjectName/src/4d.gcx" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/src/4d.gcx"
            scp "$LocalBaseDir/usermodules/$ProjectName/src/Graphics4D.h" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/src/Graphics4D.h"
            scp "$LocalBaseDir/usermodules/$ProjectName/src/Graphics4D.cpp" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/src/Graphics4D.cpp"
            scp "$LocalBaseDir/usermodules/$ProjectName/config/extmod.cmake" "${RemoteUser}@${RemoteHost}:~/micropython/extmod/extmod.cmake"
            scp "$LocalBaseDir/usermodules/$ProjectName/config/CMakeLists.txt" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/CMakeLists.txt"
            $selected = $true
            break
        }
        'b' {
            scp -r "$LocalBaseDir/usermodules/$ProjectName/src/4d.gcx" "${RemoteUser}@${RemoteHost}:~/micropython/usermodules/$ProjectName/src/4d.gcx"
            $selected = $true
            break
        }
        default {
            Write-Host "Invalid option. Please try again."
        }
    }
}

# ---------------- BUILD ------------------
$shContent = @"
#!/bin/bash
cd ~/micropython
#make -C mpy-cross
cd ports/rp2

make -j4 V=1 submodules BOARD=$TargetBoard BUILD=$BuildDirectory USER_C_MODULES=~/micropython/usermodules/$ProjectName/micropython.cmake

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
make -j4 BOARD=$TargetBoard BUILD=$BuildDirectory USER_C_MODULES=~/micropython/usermodules/$ProjectName/micropython.cmake


"@

Set-Content -Path "$LocalBaseDir/usermodules/$ProjectName/config/build_upy.sh" -Value ($shContent -replace "`r`n", "`n") -Encoding UTF8 -NoNewline
scp .\config\build_upy.sh "${RemoteUser}@${RemoteHost}:~/build_upy.sh"
ssh $RemoteUser@$RemoteHost "bash ~/build_upy.sh"

# --------- COPY BUILD FROM HOST ----------
#scp -r "${RemoteUser}@${RemoteHost}/home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}" "$LocalBaseDir/build/"

New-Item -ItemType Directory -Path "$LocalBaseDir/build/$BuildDirectory/frozen_mpy" -Force
New-Item -ItemType Directory -Path "$LocalBaseDir/build/$BuildDirectory/genhdr" -Force
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
    #Write-Host "removing remote build directory..."
    Write-Host "removing remote build directory... <- ISN'T CONFIGURED TO WORK YET"
    #ssh $RemoteUser@$RemoteHost "rm -r /home/${RemoteUser}/micropython/ports/rp2/${BuildDirectory}"<$null
}