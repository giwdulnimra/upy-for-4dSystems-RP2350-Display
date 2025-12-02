# MicroPython Port for 4D Systems RP2350 Display

This repository provides the board definition and configuration files required to run MicroPython on 4D Systems display modules powered by the Raspberry Pi RP2350 (specifically the gen4-RP2350 series).

## Port Structure and Required Files

To compile MicroPython for this specific hardware, a custom board definition directory is established inside the `ports/rp2/boards/` directory. This repository contains the specific configuration files required to map the RP2350 capabilities to the 4D Systems hardware.

Below is an explanation of the files required for this port:

### 1. `mpconfigboard.h`

This is the primary configuration header for the MicroPython firmware. It handles the specific hardware definitions for the MicroPython runtime:

- **Board Identity:** Sets the board name string seen in the REPL (e.g., `"GEN4_RP2350_70CT"`).
- **Flash Configuration:** Defines the size and parameters of the external QSPI flash memory used by the module.
- **USB Configuration:** Sets the USB Vendor ID (VID) and Product ID (PID) so the operating system recognizes the device correctly.
- **Clock Settings:** Configures the crystal frequency and PLL settings if they differ from the standard Raspberry Pi Pico reference design.

### 2. `mpconfigboard.cmake`

This CMake file directs the build system. Its primary role in this port is to override the default board header. It instructs the compiler to use the `gen4_rp2350_70ct.h` file for low-level pin definitions instead of the standard Pico header.

### 3. `pins.csv`

This file provides the mapping between the RP2350 GPIO numbers and the named pins on the board. It allows users to initialize pins in MicroPython using string identifiers that match the board's physical layout or schematic labels.

### 4. `gen4_rp2350_70ct.h` (External Requirement)

This is the C-SDK level board header file. It defines the raw macros for pin assignments, LED connections, and bus configurations used by the RP2350 C SDK.

**Important Note on `gen4_rp2350_70ct.h`:**  
This file is proprietary to the 4D Systems environment and is not distributed directly in this repository for every possible board. You can acquire it from your local installation of the 4D Workshop 5 IDE.

**Location:**  
`[Install Path]\4D Workshop 5 IDE\rp2350\boards\gen4_rp2350_70ct.h`

Please copy this file into the board definition folder before attempting to build.

## Build Instructions

To build the firmware, follow these steps assuming you have the MicroPython build toolchain and the RP2350 SDK environment set up.

1. Clone this repository.
2. Copy the board definition folder from this repository into your MicroPython `ports/rp2/boards/` directory.
3. Copy the `gen4_rp2350_70ct.h` file from the Workshop 5 IDE into that same board directory.
4. Navigate to the `ports/rp2` directory in your MicroPython source tree.
5. Run the make command specifying your board:
```bash
   make BOARD=GEN4_RP2350_70CT
```
6. Once the build completes, the firmware.uf2 file will be located in the build directory. Hold the BOOTSEL button on the display module, reset the device, and copy the UF2 file to the mount

