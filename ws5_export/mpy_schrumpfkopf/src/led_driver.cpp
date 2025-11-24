#include "led_driver.hpp"
#include "py/mphal.h"   // Für GPIO-Funktionen (mp_hal_pin_config, mp_hal_pin_write)
#include "py/mpprint.h" // Für mp_printf

LedDriver::LedDriver() : state_(false) {
    // Initialisiert den Pin als Output.
    // Dies muss mit den portspezifischen HAL-Funktionen geschehen.
    mp_hal_pin_config(ONBOARD_LED_PIN, MP_HAL_PIN_MODE_OUTPUT, MP_HAL_PIN_PULL_NONE);
    mp_hal_pin_write(ONBOARD_LED_PIN, 0); // Startet ausgeschaltet
}

void LedDriver::turn_on() {
    mp_hal_pin_write(ONBOARD_LED_PIN, 1);
    state_ = true;
}

void LedDriver::turn_off() {
    mp_hal_pin_write(ONBOARD_LED_PIN, 0);
    state_ = false;
}

bool LedDriver::is_on() const {
    return state_;
}

void LedDriver::print_status(const mp_print_t *print) const {
    mp_printf(print, "LED (Pin %d): %s", ONBOARD_LED_LED, state_ ? "AN" : "AUS");
}