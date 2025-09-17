# ---------------- CONFIG -----------------
$RemoteUser     = "armin"
$RemoteHost     = 
    "10.19.28.28"
    #"code.protronic.local"
    #"can-mon"
$LocalBaseDir = 
    "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
    #"."
# -----------------------------------------

# ------- COPY WS5-PROJEKT TO HOST --------
scp -r $LocalBaseDir"/ws5_export" "${RemoteUser}@${RemoteHost}:~/micropython/"
scp -r $LocalBaseDir"micropython/ports/rp2/boards/4DSYS_RP2350" "${RemoteUser}@${RemoteHost}:~/micropython/ports/rp2/boards"

# ---------------- BUILD ------------------
ssh $RemoteUser@$RemoteHost "
cd ~/micropython && \
make -C mpy-cross && \
cd ports/rp2 && \
make submodules && \
make clean && \
make BUILD=build-CustomTest" #erstellt Firmware für Default-Board (RPI_PICO)
#make BOARD=4DSYS_RP2350 BUILD=buid-4Dsys FIRMWARE = 4dsys_upy.uf2 USER_C_MODULES="
# copy existing Board in ports/rp2/boards/, configure:
# CMakeLists.txt (Board-spezifische Quellen)
# mpconfigboard.h / Header für Pins, Flash, Clocks
# ggf. Linker-Script (memmap.ld)

# --------- COPY BUILD FROM HOST ----------
#scp -r "${RemoteUser}@${RemoteHost}/home/armin/micropython/ports/rp2/build-CustomTest" "C:/Users/armin/OneDrive/6FS-MT-Jena_BA/Bachelorarbeit/upy_display_export"
mkdir -p "$LocalBaseDir/build-CustomTest/frozen_mpy"
mkdir -p "$LocalBaseDir/build-CustomTest/genhdr"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/*.uf2" "$LocalBaseDir/build-CustomTest/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/*.bin" "$LocalBaseDir/build-CustomTest/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/*.hex" "$LocalBaseDir/build-CustomTest/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/*.dis" "$LocalBaseDir/build-CustomTest/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/*.elf" "$LocalBaseDir/build-CustomTest/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/*.elf.map" "$LocalBaseDir/build-CustomTest/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/*.c" "$LocalBaseDir/build-CustomTest/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/frozen_mpy/*.mpy" "$LocalBaseDir/build-CustomTest/frozen_mpy/"
scp "${RemoteUser}@${RemoteHost}:/home/${RemoteUser}/micropython/ports/rp2/build-CustomTest/genhdr/*.h" "$LocalBaseDir/build-CustomTest/genhdr/"

#mkdir -p "$LocalBaseDir/build-4Dsys/frozen_mpy"
#mkdir -p "$LocalBaseDir/build-4Dsys/genhdr"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/*.uf2" "$LocalBaseDir/build-4Dsys/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/*.bin" "$LocalBaseDir/build-4Dsys/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/*.hex" "$LocalBaseDir/build-4Dsys/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/*.dis" "$LocalBaseDir/build-4Dsys/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/*.elf" "$LocalBaseDir/build-4Dsys/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/*.elf.map" "$LocalBaseDir/build-4Dsys/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/*.c" "$LocalBaseDir/build-4Dsys/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/frozen_mpy/*.mpy" "$LocalBaseDir/build-4Dsys/frozen_mpy/"
#scp "$RemoteUser@$RemoteHost:/home/${RemoteUser}/micropython/ports/rp2/build-4Dsys/genhdr/*.h" "$LocalBaseDir/build-4Dsys/genhdr/"
