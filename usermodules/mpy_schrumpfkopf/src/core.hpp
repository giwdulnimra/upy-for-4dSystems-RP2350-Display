#pragma once

#include "py/obj.h"

// *****************************************************************
// BARE METAL DEFINITIONEN (RP2040 SIO Registers)
// *****************************************************************
// Basisadresse des SIO-Peripherals
constexpr uint32_t SIO_BASE_ADDR = 0xd0000000;

// Register-Offsets
constexpr uint32_t GPIO_OUT_SET_OFFSET = 0x14; // Pin High setzen
constexpr uint32_t GPIO_OUT_CLR_OFFSET = 0x18; // Pin Low setzen
constexpr uint32_t GPIO_OUT_DIR_OFFSET = 0x20; // Pin-Richtung (Output/Input)

// Pointer Makros für den atomaren Registerzugriff
#define SIO_OUT_SET (*(volatile uint32_t *)(SIO_BASE_ADDR + GPIO_OUT_SET_OFFSET))
#define SIO_OUT_CLR (*(volatile uint32_t *)(SIO_BASE_ADDR + GPIO_OUT_CLR_OFFSET))
#define SIO_OUT_DIR (*(volatile uint32_t *)(SIO_BASE_ADDR + GPIO_OUT_DIR_OFFSET))

// *****************************************************************
// KLASSE 1: Led (mit freiem Pin)
// *****************************************************************
class Led {
public:
    // Konstruktor: Setzt den Pin und konfiguriert ihn
    Led(uint32_t pin);
    
    // Methoden (Bare Metal Steuerung)
    void turn_on();
    void turn_off();
    bool is_on() const;
    void print_status(const mp_print_t *print) const;

private:
    uint32_t pin_;
};


// *****************************************************************
// KLASSE 2: Console (für Debug-Ausgaben)
// *****************************************************************
class Console {
public:
    Console();
    // Gibt formatierten Text auf der MicroPython REPL aus
    void print_info(const char* format, ...);
    // Gibt Text auf der C-Konsole (typischerweise Debug-Ausgabe) aus
    void print_debug(const char* message) const;
};