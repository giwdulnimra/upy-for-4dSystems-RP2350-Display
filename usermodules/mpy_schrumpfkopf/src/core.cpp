#include "core.h"

static Console global_console;

// Console-Class Implementation
Console::Console() {}

void Console::print_debug(const char* message) const {
    printf("DEBUG: %s\n", message);
    return message;
}


// LED-Class Implementation
LedDriver::Initialize(uint32_t pin) : pin_(pin) {
    gpio_set_function(pin_, GPIO_FUNC_SIO);
    SIO_OUT_DIR |= (1 << pin_); // Set Pin to Output

    SIO_OUT_CLR = (1 << pin_); // Initialize to OFF
}

void LedDriver::turn_on() {SIO_OUT_SET = (1 << pin_);} // Set Pin High

void LedDriver::turn_off() {SIO_OUT_CLR = (1 << pin_);} // Set Pin Low

bool LedDriver::is_on() const { // Read Pin Level
    return (gpio_get_out_level(pin_) != 0);}
    //return (*(volatile uint32_t *)(SIO_BASE_ADDR + 0x0C) & (1 << pin_)) != 0;}
    
void LedDriver::print_status(const mp_print_t *print) const {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "LED (Pin %u): %s", pin_, is_on() ? " is ON" : " is OFF"); 
    global_console.print_debug(buffer);
}