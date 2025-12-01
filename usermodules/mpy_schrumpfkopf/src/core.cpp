#include "hardware_regs/addressmap_part.h"
#include "core.h"

#ifndef REG_WRITE
    #define REG_WRITE(addr, val) (*(volatile uint32_t*)(addr) = (val))
#endif
#ifndef REG_READ
    #define REG_READ(addr)       (*(volatile uint32_t*)(addr))
#endif

static Console global_console;

// Console-Class Implementation
Console::Console() {}

void Console::print_debug(const char* message) const {
    printf("[C++] DEBUG: %s\n", message);
}


// LED-Class Implementation
LedDriver::LedDriver(uint32_t pin) : pin_(pin) {
    uint32_t ctrl_reg = IO_BANK0_BASE + (pin_ * 8) + CTRL_OFFSET;
    REG_WRITE(ctrl_reg, GPIO_FUNC_SIO);         // ensure SIO-functionality
    REG_WRITE(SIO_BASE + SIO_GPIO_OE_SET_OFFSET, (1ul << pin_));    // output enable
    REG_WRITE(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET, (1ul << pin_));   // set low 
}

void LedDriver::turn_on() {
    REG_WRITE(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET, (1ul << pin_));
} // Set Pin High

void LedDriver::turn_off() {
    REG_WRITE(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET, (1ul << pin_));
} // Set Pin Low

uint32_t LedDriver::get_pin() const {return pin_;}

bool LedDriver::is_on() const { // Read Pin Level
    uint32_t state = REG_READ(SIO_BASE + SIO_GPIO_IN_OFFSET);
    return (state & (1ul << pin_)) != 0;
}
    
void LedDriver::print_status() const {
    char buffer[64];
    printf(buffer, sizeof(buffer), "LED (Pin %u): %s", pin_, is_on() ? " is ON" : " is OFF"); 
    global_console.print_debug(buffer);
}