#pragma once

#include <stdint.h> 
#include <stdbool.h> 
#include <stdarg.h> 

#include "hardware/regs/sio.h"
#include "hardware/addressmap.h"
#include "hardware/gpio.h"

class Console {
public:
    Console();

    void print_debug(const char* message) const;
};

class Led {
public:
    Led(uint32_t pin);
    
    void turn_on();
    void turn_off();
    bool is_on() const;
    void print_status(const mp_print_t *print) const;

private:
    uint32_t pin_;
};