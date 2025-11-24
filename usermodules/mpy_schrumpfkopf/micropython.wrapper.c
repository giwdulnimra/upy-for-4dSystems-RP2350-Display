#include "py/runtime.h"
#include "py/mphal.h"
#include "py/misc.h"
#include "led_driver.hpp"

// Forward Declaration für das Modulobjekt
extern "C" const mp_obj_module_t mp_module_cpp_hardware;

// ----------------------------------------------------
// 1. C-Objektstruktur (Instanz-Struktur)
// Diese Struktur speichert die C++-Klasseninstanz LedDriver.
typedef struct _cpp_hardware_led_obj_t {
    mp_obj_base_t base;
    LedDriver driver; // Die C++-Klasse als Instanz-Attribut
} cpp_hardware_led_obj_t;

// ----------------------------------------------------
// 2. Methoden-Implementierung (Wraps der C++-Methoden)

// Methode: led.on()
extern "C" mp_obj_t cpp_hardware_led_on(mp_obj_t self_in) {
    cpp_hardware_led_obj_t *self = (cpp_hardware_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.turn_on();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cpp_hardware_led_on_obj, cpp_hardware_led_on);

// Methode: led.off()
extern "C" mp_obj_t cpp_hardware_led_off(mp_obj_t self_in) {
    cpp_hardware_led_obj_t *self = (cpp_hardware_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.turn_off();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cpp_hardware_led_off_obj, cpp_hardware_led_off);

// Methode: led.is_on()
extern "C" mp_obj_t cpp_hardware_led_is_on(mp_obj_t self_in) {
    cpp_hardware_led_obj_t *self = (cpp_hardware_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(self->driver.is_on());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cpp_hardware_led_is_on_obj, cpp_hardware_led_is_on);


// ----------------------------------------------------
// 3. Print-Callback für Python-Ausgabe (optional, aber gewünscht)
extern "C" void cpp_hardware_led_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    cpp_hardware_led_obj_t *self = (cpp_hardware_led_obj_t*)MP_OBJ_TO_PTR(self_in);

    mp_printf(print, "<LED-Objekt ");
    self->driver.print_status(print);
    mp_printf(print, ">");
}

// ----------------------------------------------------
// 4. make_new (Konstruktor)
extern "C" mp_obj_t cpp_hardware_led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // Speicher für die Struktur reservieren. Der C++-Konstruktor wird automatisch aufgerufen!
    cpp_hardware_led_obj_t *self = (cpp_hardware_led_obj_t*)mp_obj_malloc(cpp_hardware_led_obj_t, type);
    
    // Keine weitere Initialisierung nötig, da der LedDriver-Konstruktor die Arbeit macht.
    return MP_OBJ_FROM_PTR(self);
}

// ----------------------------------------------------
// 5. Typ-Definition (Die Klasse selbst)
STATIC const mp_rom_map_elem_t cpp_hardware_led_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&cpp_hardware_led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&cpp_hardware_led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_on), MP_ROM_PTR(&cpp_hardware_led_is_on_obj) },
};
STATIC MP_DEFINE_CONST_DICT(cpp_hardware_led_locals_dict, cpp_hardware_led_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_cpp_hardware_led,
    MP_QSTR_LED,
    MP_TYPE_FLAG_NONE,
    print, cpp_hardware_led_print,
    make_new, cpp_hardware_led_make_new,
    locals_dict, &cpp_hardware_led_locals_dict
);


// ----------------------------------------------------
// 6. Modul-Registrierung (Das Modul cpp_hardware)
STATIC const mp_rom_map_elem_t cpp_hardware_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_PTR(&mp_module_cpp_hardware) },
    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&mp_type_cpp_hardware_led) }, // Klasse LED
};
STATIC MP_DEFINE_CONST_DICT(mp_module_cpp_hardware_globals, cpp_hardware_globals_table);

extern "C" const mp_obj_module_t mp_module_cpp_hardware = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_cpp_hardware_globals,
};

// Modulname in Python: cpp_hardware
MP_REGISTER_MODULE(MP_QSTR_cpp_hardware, mp_module_cpp_hardware);