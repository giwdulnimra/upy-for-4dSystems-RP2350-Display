# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     = 
    "10.19.28.28"
    #"code.protronic.local"
    #"can-mon"
$LocalBaseDir = #"."
    "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
$TargetBoard = #"4DSYS_RP2350"
    "RPI_PICO2_W"
$BuildDirectory = #"build-4Dsys"
    #"build-CustomTest"
    "build-Pico2w"
# -----------------------------------------
# ------- COPY WS5-PROJEKT TO HOST --------
scp -r $LocalBaseDir"/ws5_export" "${RemoteUser}@${RemoteHost}:~/micropython/"
scp -r $LocalBaseDir"micropython/ports/rp2/boards/4DSYS_RP2350" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/boards"

# ---------------- BUILD ------------------
ssh $RemoteUser@$RemoteHost "
cd ~/micropython && \
make -C mpy-cross && \
cd ports/rp2 && \
make BOARD=$TargetBoard submodules && \
make BOARD=$TargetBoard clean && \
make BOARD=$Targetbuild BUILD=$BuildDirectory" #erstellt Firmware für Default-Board (RPI_PICO)
#make BOARD=$Targetbuild BUILD=$BuildDirectory FIRMWARE = 4dsys_upy.uf2 USER_C_MODULES="

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
