/*
Micropython wrapper template for Graphics4D C++ library

File: micropython-Graphics4D-wrapper.cpp

WHAT THIS FILE CONTAINS
- A minimal, idiomatic MicroPython C API wrapper for a C++ class `Graphics4D`.
- Factory/constructor, destructor and example method bindings.
- Instructions / TODO markers where you should adapt method names and signatures to the actual Graphics4D API.

HOW TO USE
1. Put this file into your MicroPython port's modules/ or extmod/ directory (or wherever you place additional C/C++ modules).
2. Ensure the original Graphics4D.cpp/.h are compiled and linked into the build.
   - If Graphics4D is pure C++ make sure the port builds C++ files and links libstdc++ if necessary.
3. Add the wrapper file to the port's Makefile/BUILD rules and add the module to MICROPY_REGISTERS in the port.
4. Rebuild MicroPython.

IMPORTANT: Because I don't have the header here, this is a template. Replace the TODOs below with actual API calls (method names and parameter conversions).
*/

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

// Include the real Graphics4D header. Adjust the path as needed.
// TODO: change this include to the real header for the library you uploaded.
#include "Graphics4D.h"

// A small heap-allocated wrapper that holds a pointer to the C++ object.
typedef struct _mp_obj_graphics4d_t {
    mp_obj_base_t base;
    Graphics4D *ptr; // pointer to the real C++ object
} mp_obj_graphics4d_t;

// Constructor: Graphics4D(width, height) -- example signature, adapt to real API
STATIC mp_obj_t graphics4d_make_new(const mp_obj_type_t *type,
                                    size_t n_args, size_t n_kw,
                                    const mp_obj_t *args) {
    enum { ARG_width, ARG_height };

    mp_arg_check_num(n_args, n_kw, 0, 2, false);

    int width = 320;
    int height = 240;

    if (n_args >= 1) {
        width = mp_obj_get_int(args[0]);
    }
    if (n_args >= 2) {
        height = mp_obj_get_int(args[1]);
    }

    // allocate Python object
    mp_obj_graphics4d_t *o = m_new_obj(mp_obj_graphics4d_t);
    o->base.type = type;

    // TODO: adapt to the actual Graphics4D constructor
    // Example: o->ptr = new Graphics4D(width, height);
    try {
        o->ptr = new Graphics4D(width, height);
    } catch (...) {
        // If construction fails, raise a Python exception
        mp_raise_msg(&mp_type_RuntimeError, "Graphics4D constructor failed");
    }

    return MP_OBJ_FROM_PTR(o);
}

// Destructor / deinit method
STATIC mp_obj_t graphics4d_deinit(mp_obj_t self_in) {
    mp_obj_graphics4d_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->ptr) {
        delete self->ptr;
        self->ptr = NULL;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_deinit_obj, graphics4d_deinit);

// Example method: set_pixel(x, y, r, g, b)
// TODO: replace with real methods from your library
STATIC mp_obj_t graphics4d_set_pixel(mp_obj_t self_in, mp_obj_t x_obj, mp_obj_t y_obj, mp_obj_t color_obj) {
    mp_obj_graphics4d_t *self = MP_OBJ_TO_PTR(self_in);
    if (!self->ptr) {
        mp_raise_msg(&mp_type_RuntimeError, "Graphics4D is already deinitialized");
    }

    int x = mp_obj_get_int(x_obj);
    int y = mp_obj_get_int(y_obj);

    // Expecting color either as an int RGB or a tuple/list (r,g,b)
    int r=0,g=0,b=0;
    if (mp_obj_is_int(color_obj)) {
        // single integer: 0xRRGGBB
        mp_int_t col = mp_obj_get_int(color_obj);
        r = (col >> 16) & 0xFF;
        g = (col >> 8) & 0xFF;
        b = col & 0xFF;
    } else if (mp_obj_is_type(color_obj, &mp_type_tuple) || mp_obj_is_type(color_obj, &mp_type_list)) {
        size_t len;
        mp_obj_t *items;
        mp_obj_get_array(color_obj, &len, &items);
        if (len < 3) mp_raise_ValueError("color must have 3 components");
        r = mp_obj_get_int(items[0]);
        g = mp_obj_get_int(items[1]);
        b = mp_obj_get_int(items[2]);
    } else {
        mp_raise_TypeError("color must be int or tuple/list");
    }

    // TODO: call the real library method. Example: self->ptr->setPixel(x, y, r, g, b);
    self->ptr->setPixel(x, y, r, g, b);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_4(graphics4d_set_pixel_obj, graphics4d_set_pixel);

// Example method: fill()
STATIC mp_obj_t graphics4d_fill(mp_obj_t self_in, mp_obj_t color_obj) {
    mp_obj_graphics4d_t *self = MP_OBJ_TO_PTR(self_in);
    if (!self->ptr) mp_raise_msg(&mp_type_RuntimeError, "Graphics4D is deinitialized");

    int r=0,g=0,b=0;
    if (mp_obj_is_int(color_obj)) {
        mp_int_t col = mp_obj_get_int(color_obj);
        r = (col >> 16) & 0xFF;
        g = (col >> 8) & 0xFF;
        b = col & 0xFF;
    } else if (mp_obj_is_type(color_obj, &mp_type_tuple) || mp_obj_is_type(color_obj, &mp_type_list)) {
        size_t len;
        mp_obj_t *items;
        mp_obj_get_array(color_obj, &len, &items);
        if (len < 3) mp_raise_ValueError("color must have 3 components");
        r = mp_obj_get_int(items[0]);
        g = mp_obj_get_int(items[1]);
        b = mp_obj_get_int(items[2]);
    } else {
        mp_raise_TypeError("color must be int or tuple/list");
    }

    // TODO: call real API, e.g. self->ptr->fill(r,g,b);
    self->ptr->fill(r, g, b);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_fill_obj, graphics4d_fill);

// Example method: draw_line(x1,y1,x2,y2,color)
STATIC mp_obj_t graphics4d_draw_line(size_t n_args, const mp_obj_t *args) {
    // args[0] = self
    mp_obj_graphics4d_t *self = MP_OBJ_TO_PTR(args[0]);
    if (!self->ptr) mp_raise_msg(&mp_type_RuntimeError, "Graphics4D is deinitialized");

    if (n_args < 6) mp_raise_TypeError("draw_line(self,x1,y1,x2,y2,color)");
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    mp_obj_t color_obj = args[5];

    int r=0,g=0,b=0;
    if (mp_obj_is_int(color_obj)) {
        mp_int_t col = mp_obj_get_int(color_obj);
        r = (col >> 16) & 0xFF;
        g = (col >> 8) & 0xFF;
        b = col & 0xFF;
    } else if (mp_obj_is_type(color_obj, &mp_type_tuple) || mp_obj_is_type(color_obj, &mp_type_list)) {
        size_t len;
        mp_obj_t *items;
        mp_obj_get_array(color_obj, &len, &items);
        if (len < 3) mp_raise_ValueError("color must have 3 components");
        r = mp_obj_get_int(items[0]);
        g = mp_obj_get_int(items[1]);
        b = mp_obj_get_int(items[2]);
    } else {
        mp_raise_TypeError("color must be int or tuple/list");
    }

    // TODO: call real API, e.g. self->ptr->drawLine(x1,y1,x2,y2,r,g,b);
    self->ptr->drawLine(x1, y1, x2, y2, r, g, b);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_draw_line_obj, 6, 6, graphics4d_draw_line);

// --------------------------------------------------------------------------------
// Define locals (methods) table for the Python object
// --------------------------------------------------------------------------------
STATIC const mp_rom_map_elem_t graphics4d_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&graphics4d_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_pixel), MP_ROM_PTR(&graphics4d_set_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&graphics4d_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw_line), MP_ROM_PTR(&graphics4d_draw_line_obj) },
    // TODO: add more bindings here following the pattern above
};
STATIC MP_DEFINE_CONST_DICT(graphics4d_locals_dict, graphics4d_locals_dict_table);

// Type definition
const mp_obj_type_t graphics4d_Graphics4D_type = {
    { &mp_type_type },
    .name = MP_QSTR_Graphics4D,
    .make_new = graphics4d_make_new,
    .locals_dict = (mp_obj_dict_t*)&graphics4d_locals_dict,
};

// Module globals
STATIC const mp_rom_map_elem_t graphics4d_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_graphics4d) },
    { MP_ROM_QSTR(MP_QSTR_Graphics4D), MP_ROM_PTR(&graphics4d_Graphics4D_type) },
};
STATIC MP_DEFINE_CONST_DICT(graphics4d_module_globals, graphics4d_module_globals_table);

const mp_obj_module_t graphics4d_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&graphics4d_module_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_graphics4d, graphics4d_user_cmodule, MODULE_GRAPHICS4D_ENABLED);

/*
BUILD / INTEGRATION NOTES

- Add this file to your port's build (for example, ports/unix/modules/ or ports/<board>/modules/).
- Ensure Graphics4D.cpp/.o are compiled with the project and linked.
- If Graphics4D depends on C++ runtime, ensure your linker flags include -lstdc++ or equivalent
  and that the build supports C++ files.
- Define MODULE_GRAPHICS4D_ENABLED in build config (mpconfigport.h or via Makefile) to 1 to enable registration
  e.g. in mpconfigport.h add: #define MICROPY_PY_GRAPHICS4D (1)
  or adapt to your port's module enabling conventions.

EXAMPLE mpconfigport.h / config snippet to enable:
    // add to mpconfigport.h or your port's config
    #define MODULE_GRAPHICS4D_ENABLED (1)

MAKEFILE SNIPPET (example for unix port)

    SRC_CXX += \
        $(MPY_DIR)/extmod/micropython-Graphics4D-wrapper.cpp \
        $(MPY_DIR)/extmod/Graphics4D.cpp

    # ensure C++ linking if necessary
    LDFLAGS += -lstdc++

Troubleshooting:
- If you get C++ exceptions across C boundaries you may need to compile with -fno-exceptions or ensure proper exception handling.
- If the Graphics4D API uses overloaded methods, adapt wrapper signatures accordingly.

LICENSE / ATTRIBUTION
- This wrapper is a template. Replace and adapt as necessary.
*/
