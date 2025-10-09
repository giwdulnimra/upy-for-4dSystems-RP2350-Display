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

// Destructor / deinit method
STATIC mp_obj_t graphics4d_deinit(mp_obj_t self_in) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    if (self->gfx) {
        delete self->gfx;
        self->gfx = NULL;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_deinit_obj, graphics4d_deinit);

// Method: initialize()
STATIC mp_obj_t graphics4d_initialize(mp_obj_t self_in) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(self->gfx->Initialize());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_initialize_obj, graphics4d_initialize);

// Method: cls(draw_fb=True)
STATIC mp_obj_t graphics4d_cls(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    bool draw_fb = (n_args > 1) ? mp_obj_is_true(args[1]) : true;
    self->gfx->Cls(draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_cls_obj, 1, 2, graphics4d_cls);

// Method: rectangle_filled(x1, y1, x2, y2, color, draw_fb=True)
STATIC mp_obj_t graphics4d_rectangle_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->RectangleFilled(x1, y1, x2, y2, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_rectangle_filled_obj, 6, 7, graphics4d_rectangle_filled);


// Method: put_pixel(x, y, color, draw_fb=True)
STATIC mp_obj_t graphics4d_put_pixel(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[3]);
    bool draw_fb = (n_args > 4) ? mp_obj_is_true(args[4]) : true;
    self->gfx->PutPixel(x, y, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_put_pixel_obj, 4, 5, graphics4d_put_pixel);

// Method: line(x1, y1, x2, y2, color, draw_fb=True)
STATIC mp_obj_t graphics4d_line(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->Line(x1, y1, x2, y2, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_line_obj, 6, 7, graphics4d_line);

// Method: circle_filled(xc, yc, radius, color, draw_fb=True)
STATIC mp_obj_t graphics4d_circle_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    int xc = mp_obj_get_int(args[1]);
    int yc = mp_obj_get_int(args[2]);
    unsigned int radius = (unsigned int)mp_obj_get_int(args[3]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[4]);
    bool draw_fb = (n_args > 5) ? mp_obj_is_true(args[5]) : true;
    self->gfx->CircleFilled(xc, yc, radius, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_circle_filled_obj, 5, 6, graphics4d_circle_filled);

// Method: move_to(x, y)
STATIC mp_obj_t graphics4d_move_to(mp_obj_t self_in, mp_obj_t x_obj, mp_obj_t y_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    int x = mp_obj_get_int(x_obj);
    int y = mp_obj_get_int(y_obj);
    self->gfx->MoveTo(x, y);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(graphics4d_move_to_obj, graphics4d_move_to);

// Method: print(text, draw_fb=True)
STATIC mp_obj_t graphics4d_print(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    const char *str = mp_obj_str_get_str(args[1]);
    bool draw_fb = (n_args > 2) ? mp_obj_is_true(args[2]) : true;
    self->gfx->print(str, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_print_obj, 2, 3, graphics4d_print);


// Define locals (methods) table for the Python object
STATIC const mp_rom_map_elem_t graphics4d_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&graphics4d_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_initialize), MP_ROM_PTR(&graphics4d_initialize_obj) },
    { MP_ROM_QSTR(MP_QSTR_cls), MP_ROM_PTR(&graphics4d_cls_obj) },
    { MP_ROM_QSTR(MP_QSTR_rectangle_filled), MP_ROM_PTR(&graphics4d_rectangle_filled_obj) },
    { MP_ROM_QSTR(MP_QSTR_put_pixel), MP_ROM_PTR(&graphics4d_put_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&graphics4d_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle_filled), MP_ROM_PTR(&graphics4d_circle_filled_obj) },
    { MP_ROM_QSTR(MP_QSTR_move_to), MP_ROM_PTR(&graphics4d_move_to_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&graphics4d_print_obj) },
};
STATIC MP_DEFINE_CONST_DICT(graphics4d_locals_dict, graphics4d_locals_dict_table);

// Define the Python type object
const mp_obj_type_t graphics4d_Graphics4D_type = {
    { &mp_type_type },
    .name = MP_QSTR_Graphics4D,
    .make_new = graphics4d_make_new,
    .locals_dict = (mp_obj_dict_t*)&graphics4d_locals_dict,
};

// Define the module's globals table
STATIC const mp_rom_map_elem_t graphics4d_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_graphics4d) },
    { MP_ROM_QSTR(MP_QSTR_Graphics4D), MP_ROM_PTR(&graphics4d_Graphics4D_type) },
};
STATIC MP_DEFINE_CONST_DICT(graphics4d_module_globals, graphics4d_module_globals_table);

// Define the module object
const mp_obj_module_t graphics4d_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&graphics4d_module_globals,
};

// Register the module to make it available in Python
// The third argument is a pre-processor flag that can be used to enable/disable the module
MP_REGISTER_MODULE(MP_QSTR_graphics4d, graphics4d_user_cmodule, 1);