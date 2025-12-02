#include "core.h"
#include "mpy_glue.h"
#include <new>         // Für new

// foreward declaration of handles
struct LedWrapper { LedDriver* obj; };
struct ConsoleWrapper { Console* obj; };

static Console global_console; 

// wrap cpp-functions
extern "C" {
    LedWrapper* bridge_led_new(uint32_t pin) {
        LedWrapper* w = new LedWrapper;
        w->obj = new LedDriver(pin);
        return w;
    }
    void bridge_led_delete(LedWrapper* w) {
        if (w) {
            delete w->obj;
            delete w;
        }
    }

    void bridge_led_on(LedWrapper* w) { w->obj->turn_on(); }
    void bridge_led_off(LedWrapper* w) { w->obj->turn_off(); }
    bool bridge_led_is_on(LedWrapper* w) { return w->obj->is_on(); }
    uint32_t bridge_led_get_pin(LedWrapper* w) { return w->obj->get_pin(); }
    void bridge_led_print_status(LedWrapper* w) { w->obj->print_status(); }


    ConsoleWrapper* bridge_console_new() {
        ConsoleWrapper* w = new ConsoleWrapper;
        w->obj = &global_console;
        return w;
    }
    void bridge_console_delete(LedWrapper* w) {
        if (w) {
            delete w->obj;
            delete w;
        }
    }

    void bridge_console_info(ConsoleWrapper* w, const char* msg) {
        w->obj->print_debug(msg);
    }
}