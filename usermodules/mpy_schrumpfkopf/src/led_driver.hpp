#pragma once

#include "py/obj.h"

// Konstante für den GPIO-Pin der Onboard-LED (typischerweise Pin 25 beim Pico)
constexpr int ONBOARD_LED_PIN = 25;

// C++ Klasse für die LED-Steuerung
class LedDriver {
public:
    LedDriver();
    void turn_on();
    void turn_off();
    bool is_on() const;
    void print_status(const mp_print_t *print) const;

private:
    bool state_;
};