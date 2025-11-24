//#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "py/mpprint.h"
#include "py/misc.h"
#include <stdio.h>  // for printf

// Forward Declaration of moduleobject
extern const mp_obj_module_t mp_module_hello_world;

// Definition of calable function
//static mp_obj_t hello_world(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
static mp_obj_t hello_world_return(void) {
    const char *my_string = "'Hello World' from extern Module!";
    size_t strlen = sizeof(my_string) - 1;
    // print sring on c-side console
    printf("Hello from extern Module!\n");
    // return string to python side
    return mp_obj_new_str(my_string, strlen);
}
// register function as MicroPython object
static MP_DEFINE_CONST_FUN_OBJ_0(hello_world_return_obj, hello_world_return);

static mp_obj_t hello_world_print(void) {
    // print string on micropython console
    mp_hal_stdout_tx_str("Hello from MicroPython REPL!\n");
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(hello_world_print_obj, hello_world_print);

// Table of module contents
static const mp_rom_map_elem_t hello_module_globals_table[] = {
    // { MP_ROM_QSTR(mp_obj_new_qstr("Name im Python-Modul")), MP_ROM_PTR(Funktionsobjekt) }
    { MP_ROM_QSTR(MP_QSTR_hello_world), MP_ROM_PTR(&mp_module_hello_world) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&hello_world_return_obj) },
    { MP_ROM_QSTR(MP_QSTR_hello_world), MP_ROM_PTR(&hello_world_print_obj) },
};

// Module-Definition
static MP_DEFINE_CONST_DICT(hello_module_globals, hello_module_globals_table);

// glue code to make module object
const mp_obj_module_t mp_module_hello_world = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&hello_module_globals,
};

// make the module available to import in Python
MP_REGISTER_MODULE(MP_QSTR_hello_world, mp_module_hello_world);