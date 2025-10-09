exter "C" {
#include "py/obj.h"
#include "py/runtime.h"
}
#include "Graphics4D.h"


typedef struct _mp_obj_graphics4d_t {
    mp_obj_base_t base;
    Graphics4D *gfx; // Pointer to the C++ Graphics4D object
} mp_obj_graphics4d_t;

// Constructor for the Graphics4D class
STATIC mp_obj_t graphics4d_make_new(const mp_obj_type_t *type,
                                     size_t n_args, size_t n_kw,
                                     const mp_obj_t *args) {
    // No arguments are expected for Constructor
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // Create a new instance of the Graphics4D object
    mp_obj_graphics4d_t *self = m_new_obj(mp_obj_graphics4d_t); // Allocate memory for the object
    self->base.type = (mp_obj_type_t *)type;
    self->gfx = NULL;
    self->gfx = new Graphics4D();

    Graphics4D *native = NULL;
    native = new (std::nothrow) Graphics4D();
    if (!native) { // Error-Mapping to MicroPython
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate Graphics4D"));
    }

    self->gfx = native;
    
    bool ok = self->gfx->Initialize();
    if (!ok) {
        // cleanup and raise exception
        delete self->gfx;
        self->gfx = NULL;
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize Graphics4D"));
    }

    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(graphics4d_make_new_obj, 0, graphics4d_make_new);