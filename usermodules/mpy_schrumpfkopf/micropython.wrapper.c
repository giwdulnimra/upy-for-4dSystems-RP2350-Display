//#include "core.h"
#include "glue/mpy_glue.h"
//extern "C" {
#include "py/misc.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/runtime.h"

#include <stdio.h>          // vsnprintf

// forward declaration of module object
extern const mp_obj_module_t mp_module_core;
// forward declaration of core subclasses
extern const mp_obj_type_t mp_type_Core_LED;
extern const mp_obj_type_t mp_type_Core_Console;

// create classes for LED and Console
typedef struct _core_led_obj_t {
    mp_obj_base_t base;
    LedWrapper* led_wrapper;   // call opaque pointer to cpp-class
} core_led_obj_t;

typedef struct _core_console_obj_t {
    mp_obj_base_t base;
    ConsoleWrapper* console_wrapper;   // call opaque pointer to cpp-class
} core_console_obj_t;


/*
 * Class: LED - functions
 */
// Initialization of core.LED object: __init__(self, pin)
static mp_obj_t core_led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false); // expect exactly 1 argument (pin number)
    core_led_obj_t *self = m_new_obj(core_led_obj_t);
    self->base.type = &mp_type_Core_LED;
    uint32_t pin = mp_obj_get_int(args[0]);
    self->led_wrapper = bridge_led_new(pin);
    return MP_OBJ_FROM_PTR(self);
}

// Methode: led.on()
static mp_obj_t core_led_on(mp_obj_t self_in) {
    core_led_obj_t *self = (core_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    bridge_led_on(self->led_wrapper);
    return mp_const_none;
}
//static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(core_led_on_obj, 1, 1, core_led_on);
//static MP_DEFINE_CONST_FUN_OBJ_KW(core_led_on_obj, 1, core_led_on);
static MP_DEFINE_CONST_FUN_OBJ_1(core_led_on_obj, core_led_on);

// Methode: led.off()
static mp_obj_t core_led_off(mp_obj_t self_in) {
    core_led_obj_t *self = (core_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    bridge_led_off(self->led_wrapper);
    return mp_const_none;
}
//static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(core_led_off_obj, 1, 1, core_led_off);
//static MP_DEFINE_CONST_FUN_OBJ_KWN(core_led_off_obj, 1, core_led_off);
static MP_DEFINE_CONST_FUN_OBJ_1(core_led_off_obj, core_led_off);

// Methode: led.status()
static mp_obj_t core_led_status(mp_obj_t self_in) {
    core_led_obj_t *self = (core_led_obj_t*)MP_OBJ_TO_PTR(self_in);
    bridge_led_print_status(self->led_wrapper);
    bool led_status = bridge_led_is_on(self->led_wrapper);
    uint32_t pin = bridge_led_get_pin(self->led_wrapper);
    mp_printf(&mp_plat_print, "[MPY] Python confirms: LED (Pin %u) %s\n", pin, led_status ? " is ON" : " is OFF");
    return mp_obj_new_bool(led_status);
}
//static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(core_led_status_obj, 1, 1, core_led_status);
//static MP_DEFINE_CONST_FUN_OBJ_KW(core_led_status_obj, 1, core_led_status);
static MP_DEFINE_CONST_FUN_OBJ_1(core_led_status_obj, core_led_status);

// method-table for core.LED-class
static const mp_rom_map_elem_t core_led_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&core_led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&core_led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&core_led_status_obj) },
};
static MP_DEFINE_CONST_DICT(core_led_locals_dict, core_led_locals_dict_table);

// type  for core.LED-class
MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_Core_LED,
    MP_QSTR_led,    // class name in Python
    MP_TYPE_FLAG_NONE,
    make_new, core_led_make_new,
    locals_dict, &core_led_locals_dict
);


/*
 * Class: Console - functions
 */
// Initialization of core.Console object: __init__(self)
static mp_obj_t core_console_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    core_console_obj_t *self = (core_console_obj_t*)m_new_obj(core_console_obj_t);
    self->base.type = &mp_type_Core_Console;
    self->console_wrapper = bridge_console_new();
    return MP_OBJ_FROM_PTR(self);
}

// Methode: console.info(format, ...)
static mp_obj_t core_console_info(size_t n_args, const mp_obj_t *args) {
    core_console_obj_t *self = (core_console_obj_t*)MP_OBJ_TO_PTR(args[0]);
    mp_obj_t message_obj;
    if (n_args > 2) { // string with format args
        mp_obj_t format_string = args[1]; // extract format string
        mp_obj_t args_tuple = mp_obj_new_tuple(n_args - 2, args + 2); // extract format args
        message_obj = mp_binary_op(MP_BINARY_OP_MODULO, format_string, args_tuple);
    }
    else { //string without format args
        message_obj = args[1];
    }
    const char *msg = mp_obj_str_get_str(message_obj);
    bridge_console_info(self->console_wrapper, msg);
    mp_printf(&mp_plat_print, "[MPY]: %s \n", msg);
    /*mp_hal_stdout_tx_strn("[MPY] : ");
    mp_hal_stdout_tx_strn(msg, , strlen(msg));
    mp_hal_stdout_tx_strn("\n");*/
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(core_console_info_obj, 2, MP_OBJ_FUN_ARGS_MAX, core_console_info);

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

//} // close extern "C"
