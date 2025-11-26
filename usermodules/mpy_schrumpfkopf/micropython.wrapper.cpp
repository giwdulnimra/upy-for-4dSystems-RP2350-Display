#include "core.h"

#include "py/runtime.h"
#include "py/obj.h"
#include "py/mphal.h"
#include "py/misc.h"

#include <cstdio>          // vsnprintf

// forward declaration of module object
extern const mp_obj_module_t mp_module_cpp_led;
extern const mp_obj_module_t mp_module_console;


// Class LED - create a type for the LED class
typedef struct _cpp_led_obj_t {
    mp_obj_base_t base;
    LedDriver driver;
} cpp_led_obj_t;

// Initialize LED object
extern "C" mp_obj_t cpp_led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false); // expect exactly 1 argument (pin number)
    cpp_led_obj_t *self = m_new_obj(cpp_led_obj_t);
    self->base.type = &mp_type_cpp_led;
    uint32_t pin = mp_obj_get_int(args[0]);
    new (&self->driver) Initialize(pin);

    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(cpp_led_make_new_obj, 1, cpp_led_make_new);

// Methode: led.on()
extern "C" mp_obj_t cpp_led_on(mp_obj_t self_in) {
    cpp_led_obj_t *self = (cpp_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.turn_on();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cpp_led_on_obj, cpp_led_on);

// Methode: led.off()
extern "C" mp_obj_t cpp_led_off(mp_obj_t self_in) {
    cpp_led_obj_t *self = (cpp_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.turn_off();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cpp_led_off_obj, cpp_led_off);

// Methode: led.status()
extern "C" mp_obj_t cpp_led_status(mp_obj_t self_in) {
    cpp_led_obj_t *self = (cpp_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.print_status(&mp_plat_print);
    return mp_obj_new_bool(self->driver.is_on());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cpp_led_status_obj, cpp_led_status);


STATIC const mp_rom_map_elem_t cpp_led_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&cpp_led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&cpp_led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&cpp_led_status_obj) },
};
STATIC MP_DEFINE_CONST_DICT(cpp_led_locals_dict, cpp_led_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_cpp_led,
    MP_QSTR_LED,
    MP_TYPE_FLAG_NONE,
    make_new, cpp_led_make_new,
    locals_dict, &cpp_led_locals_dict
);

STATIC const mp_rom_map_elem_t cpp_led_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_PTR(&mp_module_cpp_led) },
    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&mp_type_cpp_led) }, // LED-class
};
STATIC MP_DEFINE_CONST_DICT(mp_module_cpp_led_globals, cpp_led_globals_table);

extern "C" const mp_obj_module_t mp_module_cpp _led= {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_cpp_led_globals,
};

// Modulname in Python: cpp_led
MP_REGISTER_MODULE(MP_QSTR_cpp_led, mp_module_cpp_led);


// Class Console - create a type for the Console class
typedef struct _console_obj_t {
    mp_obj_base_t base;
    Console console;
} console_obj_t;

// Initialize Console object
extern "C" mp_obj_t console_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    console_obj_t *self = (console_obj_t*)m_new_obj(console_obj_t);
    self->base.type = &mp_type_Console;
    new (&self->console) Console();
    return MP_OBJ_FROM_PTR(self);
}

// Methode: console.info(format, ...)
extern "C" mp_obj_t console_info(size_t n_args, const mp_obj_t *args) {
    console_obj_t *self = (console_obj_t*)MP_OBJ_TO_PTR(args[0]);
    const char *format_str = mp_obj_str_get_str(args[1]);
    char buffer[256]; // Stack-Buffer
    vsnprintf(buffer, sizeof(buffer), format_str, (const char *)args + 2 * sizeof(mp_obj_t)); // Format string with variable arguments
    int len = strlen(buffer);

    if (len > 0 && len < (int)sizeof(buffer)) {
        self->console.print_debug(buffer);
        mp_hal_stdout_tx_strn(buffer, len);
        mp_hal_stdout_tx_strn("\n", 1);
    }
    else {
        mp_hal_tx_strn("BUFFER_OVERFLOW");
        mp_hal_stdout_tx_strn("\n", 1);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(console_info_obj, 2, MP_OBJ_FUN_ARGS_MAX, console_info);

// Methodentabelle für Console
STATIC const mp_rom_map_elem_t console_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&console_info_obj) },
};
STATIC MP_DEFINE_CONST_DICT(console_locals_dict, console_locals_dict_table);

// Typ-Definition (Klasse) für Console
MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_Console,
    MP_QSTR_Console,
    MP_TYPE_FLAG_NONE,
    make_new, console_make_new,
    locals_dict, &console_locals_dict
);

STATIC const mp_rom_map_elem_t console_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_PTR(&mp_module_console) },
    { MP_ROM_QSTR(MP_QSTR_Console), MP_ROM_PTR(&mp_type_Console) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_console_globals, console_globals_table);

const mp_obj_module_t mp_module_console = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_console_globals,
};

// Modulname in Python: console
MP_REGISTER_MODULE(MP_QSTR_console, mp_module_console);
