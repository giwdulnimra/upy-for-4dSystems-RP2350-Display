#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mpprint.h"
#include "py/misc.h"
#include <stdio.h>

// Forward Declaration für das Modulobjekt
extern const mp_obj_module_t mp_module_simple_hello;

// ---------------- 1. STRUKTUR DEFINIEREN ----------------
typedef struct _example_MinimalPrint_obj_t {
    mp_obj_base_t base; 
} example_MinimalPrint_obj_t;

// ---------------- 2. MAKE_NEW (KONSTRUKTOR) ----------------
static mp_obj_t example_MinimalPrint_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    example_MinimalPrint_obj_t *self = mp_obj_malloc(example_MinimalPrint_obj_t, type);
    return MP_OBJ_FROM_PTR(self);
}

// ---------------- 3. PRINT CALLBACK IMPLEMENTIEREN ----------------
// Die Funktion, die __repr__ und __str__ steuert.
static void example_MinimalPrint_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_uint_t elapsed_simulated = 90000; 

    mp_printf(print, "<MinimalPrintObject: %q", (qstr)MP_QSTR_MinimalPrint);

    if (kind == PRINT_STR) {
        mp_printf(print, " wurde vor %d Sekunden initialisiert", (int)(elapsed_simulated / 1000));
    }
    
    mp_printf(print, ">");
}

// ---------------- 4. OBJEKTTYP DEFINIEREN ----------------
MP_DEFINE_CONST_OBJ_TYPE(
    example_type_MinimalPrint,
    MP_QSTR_MinimalPrint,
    MP_TYPE_FLAG_NONE,
    print, example_MinimalPrint_print,
    make_new, example_MinimalPrint_make_new
);

// ---------------- 5. MODUL GLUE CODE ----------------
static const mp_rom_map_elem_t simple_hello_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_simple_hello) },
    // Registriert die Klasse im Modul
    { MP_ROM_QSTR(MP_QSTR_MinimalPrint), MP_ROM_PTR(&example_type_MinimalPrint) },
};

static MP_DEFINE_CONST_DICT(simple_hello_module_globals, simple_hello_module_globals_table);

const mp_obj_module_t mp_module_simple_hello = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&simple_hello_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_simple_hello, mp_module_simple_hello);