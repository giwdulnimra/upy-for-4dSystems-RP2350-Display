exter "C" {
#include "py/obj.h"
#include "py/runtime.h"
}
#include "Graphics4D.h"

// Define all functions of the module here
// Struktur für das Python-Objekt
typedef struct _mp_obj_graphics4d_t {
    mp_obj_base_t base;
    Graphics4D* gfx; // Pointer to the C++ Graphics4D object
} mp_obj_graphics4d_t;
//??????????????????????????????????????????????????????????











using namespace mp;
// init() 
STATIC mp_obj_t mp_graphics4d_init(void) {
    bool ok = Graphics4D::GetInstance().Initialize();
    return mp_obj_new_bool(ok);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(graphics4d_init_obj, mp_graphics4d_init);

// cls()
STATIC mp_obj_t mp_graphics4d_cls(mp_obj_t draw_fb) {
    Graphics4D::GetInstance().Cls(mp_obj_is_true(draw_fb));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_cls_obj, mp_graphics4d_cls);

// rect_filled(x1, y1, x2, y2, color, fb)
STATIC mp_obj_t mp_graphics4d_rectfilled(mp_obj_t x1, mp_obj_t y1, mp_obj_t x2, mp_obj_t y2, mp_obj_t color, mp_obj_t fb) {
    auto &g = Graphics4D::GetInstance();
    g.RectangleFilled(mp_obj_get_int(x1), mp_obj_get_int(y1), mp_obj_get_int(x2), mp_obj_get_int(y2), (uint16_t)mp_obj_get_int(color), mp_obj_is_true(fb));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_6(graphics4d_rectfilled_obj, mp_graphics4d_rectfilled);

// pixel(x, y, color[, alpha][, fb])
STATIC mp_obj_t mp_graphics4d_pixel(size_t n, const mp_obj_t *args) {
    int x = mp_obj_get_int(args[0]);
    int y = mp_obj_get_int(args[1]);
    uint16_t col = mp_obj_get_int(args[2]);
    uint8_t alpha = (n>3 ? mp_obj_get_int(args[3]) : 255);
    bool fb = (n>4 && mp_obj_is_true(args[4]));
    if (alpha == 255) {
        Graphics4D::GetInstance().PutPixel(x,y,col, fb); }
    else {
        Graphics4D::GetInstance().PutPixel(x,y,col,alpha, fb); }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_pixel_obj, 3, 5, mp_graphics4d_pixel);


// locals dict
STATIC const mp_rom_map_elem_t graphics4d_locals[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&graphics4d_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_cls), MP_ROM_PTR(&graphics4d_cls_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect_filled), MP_ROM_PTR(&graphics4d_rectfilled_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&graphics4d_pixel_obj) },
};
STATIC MP_DEFINE_CONST_DICT(graphics4d_locals_dict, graphics4d_locals);

// module
const mp_obj_module_t graphics4d_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&graphics4d_locals_dict, 
};
MP_REGISTER_MODULE(MP_QSTR_graphics4d, graphics4d_module, 1);