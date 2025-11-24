#include "py/obj.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mperrno.h"

// Include your Graphics4D headers
#include "Graphics4D.h"

typedef struct _graphics4d_obj_t {
    mp_obj_base_t base;
    Graphics4D *gfx;
    GraphicsMedia4D *media;
} graphics4d_obj_t;

// Global instances (these should match your existing C++ code)
Graphics4D &gfx = Graphics4D::GetInstance();
GraphicsMedia4D &img = GraphicsMedia4D::GetInstance();

// Constructor
mp_obj_t graphics4d_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    
    graphics4d_obj_t *self = m_new_obj(graphics4d_obj_t);
    self->base.type = &graphics4d_type;
    self->gfx = &gfx;
    self->media = &img;
    
    return MP_OBJ_FROM_PTR(self);
}

// Initialize display
mp_obj_t graphics4d_initialize(mp_obj_t self_in) {
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool result = self->gfx->Initialize();
    return mp_obj_new_bool(result);
}

// Clear screen
mp_obj_t graphics4d_cls(mp_obj_t self_in) {
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->gfx->Cls();
    return mp_const_none;
}

// Draw rectangle
mp_obj_t graphics4d_rectangle(size_t n_args, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_args, 5, 6, false);
    
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    
    uint16_t color = 0xFFFF; // white default
    if (n_args > 5) {
        color = mp_obj_get_int(args[5]);
    }
    
    self->gfx->Rectangle(x1, y1, x2, y2, color, true);
    return mp_const_none;
}

// Draw filled rectangle
mp_obj_t graphics4d_rectangle_filled(size_t n_args, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_args, 5, 6, false);
    
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    
    uint16_t color = 0xFFFF; // white default
    if (n_args > 5) {
        color = mp_obj_get_int(args[5]);
    }
    
    self->gfx->RectangleFilled(x1, y1, x2, y2, color, true);
    return mp_const_none;
}

// Draw circle
mp_obj_t graphics4d_circle(size_t n_args, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_args, 4, 5, false);
    
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    int radius = mp_obj_get_int(args[3]);
    
    uint16_t color = 0xFFFF; // white default
    if (n_args > 4) {
        color = mp_obj_get_int(args[4]);
    }
    
    self->gfx->Circle(x, y, radius, color, true);
    return mp_const_none;
}

// Draw line
mp_obj_t graphics4d_line(size_t n_args, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_args, 5, 6, false);
    
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    
    uint16_t color = 0xFFFF; // white default
    if (n_args > 5) {
        color = mp_obj_get_int(args[5]);
    }
    
    self->gfx->Line(x1, y1, x2, y2, color, true);
    return mp_const_none;
}

// Put pixel
mp_obj_t graphics4d_put_pixel(size_t n_args, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_args, 3, 4, false);
    
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    
    uint16_t color = 0xFFFF; // white default
    if (n_args > 3) {
        color = mp_obj_get_int(args[3]);
    }
    
    self->gfx->PutPixel(x, y, color, true);
    return mp_const_none;
}

// Set backlight
mp_obj_t graphics4d_set_backlight(mp_obj_t self_in, mp_obj_t level) {
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint16_t level_val = mp_obj_get_int(level);
    self->gfx->SetBacklightLevel(level_val);
    return mp_const_none;
}

// Set contrast
mp_obj_t graphics4d_set_contrast(mp_obj_t self_in, mp_obj_t level) {
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t level_val = mp_obj_get_int(level);
    self->gfx->Contrast(level_val);
    return mp_const_none;
}

// Get width
mp_obj_t graphics4d_get_width(mp_obj_t self_in) {
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint width = self->gfx->GetWidth();
    return mp_obj_new_int(width);
}

// Get height
mp_obj_t graphics4d_get_height(mp_obj_t self_in) {
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint height = self->gfx->GetHeight();
    return mp_obj_new_int(height);
}

// Load image control from file
mp_obj_t graphics4d_load_image_control(size_t n_args, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_args, 2, 3, false);
    
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    const char *filename = mp_obj_str_get_str(args[1]);
    
    int form_index = 0;
    if (n_args > 2) {
        form_index = mp_obj_get_int(args[2]);
    }
    
    ImageControl4D handle = self->media->LoadImageControl(filename, form_index);
    if (handle == NULL) {
        mp_raise_OSError(MP_ENOENT);
    }
    
    // Store handle in an integer (you might want to create a proper handle object)
    return mp_obj_new_int((mp_int_t)handle);
}

// Show image
mp_obj_t graphics4d_show_image(mp_obj_t self_in, mp_obj_t handle, mp_obj_t index) {
    graphics4d_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ImageControl4D img_handle = (ImageControl4D)mp_obj_get_int(handle);
    uint img_index = mp_obj_get_int(index);
    
    self->media->Show(img_handle, img_index, true);
    return mp_const_none;
}

// Method table
STATIC const mp_rom_map_elem_t graphics4d_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_initialize), MP_ROM_PTR(&graphics4d_initialize_obj) },
    { MP_ROM_QSTR(MP_QSTR_cls), MP_ROM_PTR(&graphics4d_cls_obj) },
    { MP_ROM_QSTR(MP_QSTR_rectangle), MP_ROM_PTR(&graphics4d_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_rectangle_filled), MP_ROM_PTR(&graphics4d_rectangle_filled_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&graphics4d_circle_obj) },
    { MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&graphics4d_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&graphics4d_put_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_backlight), MP_ROM_PTR(&graphics4d_set_backlight_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_contrast), MP_ROM_PTR(&graphics4d_set_contrast_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&graphics4d_get_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&graphics4d_get_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_image), MP_ROM_PTR(&graphics4d_load_image_control_obj) },
    { MP_ROM_QSTR(MP_QSTR_show_image), MP_ROM_PTR(&graphics4d_show_image_obj) },
};

STATIC MP_DEFINE_CONST_DICT(graphics4d_locals_dict, graphics4d_locals_dict_table);

// Define the type
const mp_obj_type_t graphics4d_type = {
    { &mp_type_type },
    .name = MP_QSTR_Graphics4D,
    .make_new = graphics4d_make_new,
    .locals_dict = (mp_obj_dict_t*)&graphics4d_locals_dict,
};

// Define module globals
STATIC const mp_map_elem_t graphics4d_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_graphics4d) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Graphics4D), (mp_obj_t)&graphics4d_type },
    
    // Color constants
    { MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_INT(0x0000) },
    { MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_INT(0xFFFF) },
    { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(0xF800) },
    { MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(0x07E0) },
    { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(0x001F) },
    { MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_INT(0xFFE0) },
    { MP_ROM_QSTR(MP_QSTR_CYAN), MP_ROM_INT(0x07FF) },
    { MP_ROM_QSTR(MP_QSTR_MAGENTA), MP_ROM_INT(0xF81F) },
};

STATIC MP_DEFINE_CONST_DICT(graphics4d_globals, graphics4d_globals_table);

// Define module object
const mp_obj_module_t graphics4d_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&graphics4d_globals,
};

// Register the module
MP_REGISTER_MODULE(MP_QSTR_graphics4d, graphics4d_module);