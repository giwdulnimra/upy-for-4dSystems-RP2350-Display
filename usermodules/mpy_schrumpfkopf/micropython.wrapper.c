#include "core.h"
extern "C" {
#include "py/runtime.h"
#include "py/obj.h"
#include "py/mphal.h"
#include "py/misc.h"

#include <cstdio>          // vsnprintf

// forward declaration of module object
extern const mp_obj_module_t mp_module_core;
// forward declaration of core subclasses
extern const mp_obj_type_t mp_type_Core_LED;
extern const mp_obj_type_t mp_type_Core_Console;


/*
 * Class LED - create a type for the LED class
 */
typedef struct _core_led_obj_t {
    mp_obj_base_t base;
    LedDriver driver;   // embed C++ class
} core_led_obj_t;

// Initialization of core.LED object: __init__(self, pin)
//static mp_obj_t core_led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
extern "C" mp_obj_t core_led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false); // expect exactly 1 argument (pin number)
    core_led_obj_t *self = m_new_obj(core_led_obj_t);
    self->base.type = &mp_type_core_LED;
    uint32_t pin = mp_obj_get_int(args[0]);
    // call C++ constructor with placement new
    new (&self->driver) Initialize(pin);

    return MP_OBJ_FROM_PTR(self);
}
//static MP_DEFINE_CONST_FUN_OBJ_KW(core_led_make_new_obj, 1, core_led_make_new);

// Methode: led.on()
//static mp_obj_t core_led_on(mp_obj_t self_in) {
extern "C" mp_obj_t core_led_on(mp_obj_t self_in) {
    core_led_obj_t *self = (core_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.turn_on();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(core_led_on_obj, core_led_on);

// Methode: led.off()
//static mp_obj_t core_led_off(mp_obj_t self_in) {
extern "C" mp_obj_t core_led_off(mp_obj_t self_in) {
    core_led_obj_t *self = (core_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.turn_off();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(core_led_off_obj, core_led_off);

// Methode: led.status()
//static mp_obj_t core_led_status(mp_obj_t self_in) {
extern "C" mp_obj_t core_led_status(mp_obj_t self_in) {
    core_led_obj_t *self = (core_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    self->driver.print_status();
    bool led_status = self->driver.is_on();
    uint32_t pin = self->driver.get_pin();
    mp_printf(&mp_plat_print, "[MPY] Python confirms: LED (Pin %u) %s\n", pin, led_status ? " is ON" : " is OFF");
    return mp_obj_new_bool(led_status);
}
static MP_DEFINE_CONST_FUN_OBJ_1(core_led_status_obj, core_led_status);   
    

    // SCHRITT 3: is_on zurückgeben (als Python Boolean)
    return mp_obj_new_bool(is_active);
// method-table for core.LED-class
static const mp_rom_map_elem_t core_led_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&core_led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&core_led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&core_led_status_obj) },
};
static MP_DEFINE_CONST_DICT(core_led_locals_dict, core_led_locals_dict_table);

// type  for core.LED-class
MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_core_LED,
    MP_QSTR_led,    // class name in Python
    MP_TYPE_FLAG_NONE,
    make_new, core_led_make_new,
    locals_dict, &core_led_locals_dict
);


/*
 * Class Console - create a type for the Console class
 */
typedef struct _console_obj_t {
    mp_obj_base_t base;
    Console console;   // embed C++ class
} console_obj_t;

// Initialization of core.Console object: __init__(self)
//static mp_obj_t core_console_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
extern "C" mp_obj_t core_console_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    core_console_obj_t *self = (core_console_obj_t*)m_new_obj(core_console_obj_t);
    self->base.type = &mp_type_Core_Console;
    new (&self->console) Console();
    return MP_OBJ_FROM_PTR(self);
}
//static MP_DEFINE_CONST_FUN_OBJ_KW(core_console_make_new_obj, 1, core_console_make_new);

// Methode: console.info(format, ...)
extern "C" mp_obj_t console_info(size_t n_args, const mp_obj_t *args) {
    core_console_obj_t *self = (core_console_obj_t*)MP_OBJ_TO_PTR(args[0]);
    const char *format_str = mp_obj_str_get_str(args[1]); // extractr format string
    char buffer[256]; // Stack-Buffer
    vsnprintf(buffer, sizeof(buffer), format_str, (const char *)args + 2 * sizeof(mp_obj_t)); // Format string with variable arguments
    int len = strlen(buffer);

    if (len > 0 && len < (int)sizeof(buffer)) {
        self->console.print_debug(buffer);
        mp_hal_stdout_tx_strn(buffer, len);
        mp_hal_stdout_tx_strn("\n", 1);
    }
    else {
        mp_hal_stdout_tx_strn("BUFFER_OVERFLOW");
        mp_hal_stdout_tx_strn("\n", 1);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(core_console_info_obj, 2, MP_OBJ_FUN_ARGS_MAX, core_console_info);

// Methodentabelle für Console
static const mp_rom_map_elem_t console_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&console_info_obj) },
};
static MP_DEFINE_CONST_DICT(console_locals_dict, console_locals_dict_table);

// method-table for core.Console-class
static const mp_rom_map_elem_t core_console_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&core_console_info_obj) },
};
static MP_DEFINE_CONST_DICT(core_console_locals_dict, core_console_locals_dict_table);

// type definition for core.Console-class
MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_Core_Console,
    MP_QSTR_console,    // class name in Python
    MP_TYPE_FLAG_NONE,
    make_new, core_console_make_new,
    locals_dict, &core_console_locals_dict
);


/*
 * include "led" and "console" classes into "core" module
 */
// set globals-table of core-module: contains class-objects
static const mp_rom_map_elem_t core_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_core) },
    { MP_ROM_QSTR(MP_QSTR_led), MP_ROM_PTR(&mp_type_Core_LED) },         // core.led
    { MP_ROM_QSTR(MP_QSTR_console), MP_ROM_PTR(&mp_type_Core_Console) }, // core.console
};
static MP_DEFINE_CONST_DICT(mp_module_core_globals, core_module_globals_table);

// module-definition
const mp_obj_module_t mp_module_core = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_core_globals,
};

// register module for Python import: import core
MP_REGISTER_MODULE(MP_QSTR_core, mp_module_core);

} // close extern "C"
