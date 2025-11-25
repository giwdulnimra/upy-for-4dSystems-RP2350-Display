#include "core.hpp"
#include "py/mpprint.h"
#include "py/mphal.h"
#include "hardware/gpio.h" // gpio_set_function
#include <cstdio>          // vsnprintf


static Console global_console;

// Console-Class Implementation
Console::Console() {}

void Console::print_debug(const char* message) const {
    printf("DEBUG: %s\n", message);
}

void Console::print_info(const char* format_message, ...) { // function takes 1 fixed + n variable args, format string first 
    char buffer[256]; // Stack-Buffer
    va_list args; // converts variable arguments to a list

    va_start(args, format_message); // formats string with variable arguments
    int len = vsnprintf(buffer, sizeof(buffer), format_message, args);
    va_end(args); // final string is now in buffer

    if (len > 0 && len < (int)sizeof(buffer)) {
        this->print_debug(buffer);
        mp_hal_stdout_tx_strn(buffer, len);
        mp_hal_stdout_tx_strn("\n", 1);
    }
    else {
        this->print_debug("BUFFER_OVERFLOW");
    }
}


// LED-Class Implementation
Led::Led(uint32_t pin) : pin_(pin) {
    gpio_set_function(pin_, GPIO_FUNC_SIO);
    SIO_OUT_DIR |= (1 << pin_); // Set Pin to Output

    SIO_OUT_CLR = (1 << pin_); // Initialize to OFF
}

void Led::turn_on() {SIO_OUT_SET = (1 << pin_);} // Set Pin High

void Led::turn_off() {SIO_OUT_CLR = (1 << pin_);} // Set Pin Low

bool Led::is_on() const { // Read Pin Level
    return (gpio_get_out_level(pin_) != 0);}
    //return (*(volatile uint32_t *)(SIO_BASE_ADDR + 0x0C) & (1 << pin_)) != 0;}
    
void Led::print_status(const mp_print_t *print) const {
    global_console.print_debug("LED (Pin %u): %s", pin_, is_on() ? " is ON" : " is OFF");

}