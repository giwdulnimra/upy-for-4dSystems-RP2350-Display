#include "Graphics4D.h"
#include <new> // For std::nothrow
//extern "C" {
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "py/mpprint.h"
//#include "py/misc.h"
//}

// Forward declarations for type objects
extern const mp_obj_type_t graphics4d_Graphics4D_type;
extern const mp_obj_type_t graphics4d_Touch4D_type;
extern const mp_obj_type_t graphics4d_TextArea_type;
extern const mp_obj_type_t graphics4d_ImageControl_type;

/*extern const uint8_t Font1[];
extern const uint8_t Font2[];
extern const uint8_t Font3[];
extern const uint8_t Font4[];*/

/*
TEXTAREA4D
*/
typedef struct _mp_obj_textarea_t {
    mp_obj_base_t base;
    TextArea *ta; // Zeiger auf die native C++-Instanz
} mp_obj_textarea_t;

/*// Factory-Funktion für TextArea
extern "C" mp_obj_t mp_textarea_make_new(size_t n_args, const mp_obj_t *args) {
    if (n_args != 6) {
        mp_raise_TypeError(MP_ERROR_TEXT("CreateTextArea(x1, y1, x2, y2, fg, bg)"));
    }
    int x1 = mp_obj_get_int(args[0]);
    int y1 = mp_obj_get_int(args[1]);
    int x2 = mp_obj_get_int(args[2]);
    int y2 = mp_obj_get_int(args[3]);
    uint16_t fg = (uint16_t)mp_obj_get_int(args[4]);
    uint16_t bg = (uint16_t)mp_obj_get_int(args[5]);
    mp_obj_textarea_t *o = m_new_obj(mp_obj_textarea_t);
    o->base.type = (mp_obj_type_t *)&graphics4d_TextArea_type;
    o->ta = Graphics4D::GetInstance().CreateTextArea(x1, y1, x2, y2, fg, bg);
    if (!o->ta) {
        m_del_obj(mp_obj_textarea_t, o);
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("CreateTextArea failed"));
    }
    return MP_OBJ_FROM_PTR(o);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_textarea_make_new_obj, 6, 6, mp_textarea_make_new);*/

// deinit / close method
extern "C" mp_obj_t mp_textarea_deinit(size_t n_args, const mp_obj_t *args) {
    mp_obj_textarea_t *self = static_cast<mp_obj_textarea_t*>(MP_OBJ_TO_PTR(args[0]));
    if (self->ta) {
        delete self->ta;
        self->ta = nullptr;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_textarea_deinit_obj, 1, 1, mp_textarea_deinit);

// Define locals(methods)-table for the TextArea4D Python object
static const mp_rom_map_elem_t mp_textarea_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mp_textarea_deinit_obj) },
};
static MP_DEFINE_CONST_DICT(mp_textarea_locals, mp_textarea_locals_table);

// Define the Python type object for TextArea4D
const mp_obj_type_t graphics4d_TextArea_type = {
    .base = { &mp_type_type },
    .name = MP_QSTR_TextArea,
    //.make_new = NULL, // Konstruktion nur über Factory
    .locals_dict = (mp_obj_dict_t*)&mp_textarea_locals, //?
};


/*
IMAGECONTROL
*/
typedef struct _mp_obj_imagecontrol_t {
    mp_obj_base_t base;
    ImageControl4D hndl; // Zeiger auf die C++ ImageControl-Instanz
} mp_obj_imagecontrol_t;
//###

// Define the Python type object for ImageControl
const mp_obj_type_t graphics4d_ImageControl_type = {
    .base = { &mp_type_type },
    .name = MP_QSTR_ImageControl,
    //.make_new = NULL, // Konstruktion nur über Factory
    //.locals_dict = (mp_obj_dict_t*)&mp_imagecontrol_locals,
};

/*
GRAPHICS4D
*/
typedef struct _mp_obj_graphics4d_t {
    mp_obj_base_t base;
    Graphics4D *gfx; // Pointer to the C++ Graphics4D object
} mp_obj_graphics4d_t;

// Constructor for the Graphics4D class - graphics4d.init()
extern "C" mp_obj_t mp_graphics4d_make_new(const mp_obj_type_t *type,
                                     size_t n_args, size_t n_kw,
                                     const mp_obj_t *args) {
    // No arguments are expected for Constructor
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    // Create a new instance of the Graphics4D object
    mp_obj_graphics4d_t *self = m_new_obj(mp_obj_graphics4d_t); // Allocate memory for the object
    self->base.type = (mp_obj_type_t *)type;
    Graphics4D *native = &Graphics4D::GetInstance();
    // Secure-check Memory allocation
    if (!native) { // Error-Mapping to MicroPython
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Failed to allocate Graphics4D"));
    }
    self->gfx = native;
    bool ok = self->gfx->Initialize();
    if (!ok) {
        // cleanup and raise exception
        //delete self->gfx;
        (void)self;
        //self->gfx = NULL;
        self->gfx = nullptr;
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize Graphics4D"));
    }
    return MP_OBJ_FROM_PTR(self);
}
/*static MP_DEFINE_CONST_FUN_OBJ_KW(mp_graphics4d_make_new_obj, 0, mp_graphics4d_make_new);*/

// Destructor / deinit method
extern "C" mp_obj_t mp_graphics4d_deinit(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    if (self->gfx) {
        //delete self->gfx;
        (void)self;
        //self->gfx = NULL;
        self->gfx = nullptr;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_deinit_obj, 1, 1, mp_graphics4d_deinit);
// close()-alias for deinit
extern "C" mp_obj_t mp_graphics4d_close(size_t n_args, const mp_obj_t *args) {
    return mp_graphics4d_deinit(1, args);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_close_obj, 1, 1, mp_graphics4d_close);

// convenience guard macro for wrappers
#define GFX_CHECK(self)     if (!self || !(self)->gfx) {mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Graphics4D not initialized or closed properly"));}

// Method: DrawWidget(num, f, x, y, gci_array)
extern "C" mp_obj_t mp_graphics4d_draw_widget(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    if (n_args != 6) {
        mp_raise_TypeError(MP_ERROR_TEXT("draw_widget(num, f, x, y, gci_array)"));
    }
    // Extract integer arguments (args[1] to args[4])
    int num = mp_obj_get_int(args[1]);
    int f = mp_obj_get_int(args[2]);
    int x = mp_obj_get_int(args[3]);
    int y = mp_obj_get_int(args[4]);
    // Extract the buffer argument (gci_array) at args[5]
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[5], &bufinfo, MP_BUFFER_READ);
    // Call the underlying C++ function
    self->gfx->DrawWidget(num, f, x, y, (const uint8_t *)bufinfo.buf);    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_draw_widget_obj, 6, 6, mp_graphics4d_draw_widget);

// ~ SetFrameBuffer
// ~ Initialize
// ~ Reset

// Method: BlendColor(base_color, new_color, alpha)
extern "C" mp_obj_t mp_graphics4d_blend_color(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    uint16_t base_color = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t new_color = (uint16_t)mp_obj_get_int(args[2]);
    uint8_t alpha = (uint8_t)mp_obj_get_int(args[3]);
    uint16_t result_color = self->gfx->BlendColor(base_color, new_color, alpha);
    return mp_obj_new_int(result_color);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_blend_color_obj, 4, 4, mp_graphics4d_blend_color);


// Method: getWidth()
extern "C" mp_obj_t mp_graphics4d_get_width(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    uint width = self->gfx->GetWidth();
    return mp_obj_new_int(width);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_get_width_obj, 1, 1, mp_graphics4d_get_width);

// Method: getHeight()
extern "C" mp_obj_t mp_graphics4d_get_height(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    uint height = self->gfx->GetHeight();
    return mp_obj_new_int(height);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_get_height_obj, 1, 1, mp_graphics4d_get_height);

// Method: setBacklightLevel(int)
extern "C" mp_obj_t mp_graphics4d_set_backlight_level(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int level = mp_obj_get_int(args[1]);
    self->gfx->SetBacklightLevel(level);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_set_backlight_level_obj, 2, 2, mp_graphics4d_set_backlight_level);

// Method: setContrast(int)
extern "C" mp_obj_t mp_graphics4d_contrast(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int contrast = mp_obj_get_int(args[1]);
    self->gfx->Contrast(contrast);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_contrast_obj, 2, 2, mp_graphics4d_contrast);

// Method: Screenmode(orientation)
extern "C" mp_obj_t mp_graphics4d_screenmode(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int mode = mp_obj_get_int(args[1]);
    int last_orientation = self->gfx->ScreenMode(mode);
    return mp_obj_new_int(last_orientation);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_screenmode_obj, 2, 2, mp_graphics4d_screenmode);

// ~ SetAddressWindow
// ~ SendFrameBuffer
// ~ GetFrameBuffer

// Method: setBackgroundColor(new_color)
extern "C" mp_obj_t mp_graphics4d_set_background_color(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    uint16_t color = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t old_color = self->gfx->SetBackgroundColor(color);
    return mp_obj_new_int(old_color);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_set_background_color_obj, 2, 2, mp_graphics4d_set_background_color);

// Method: ClipWindow(x1, y1, x2, y2) || ClipWindow() -> reset
extern "C" mp_obj_t mp_graphics4d_clip_window(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    if (n_args == 1) {
        self->gfx->ClipWindow(0, 0, self->gfx->GetWidth() - 1, self->gfx->GetHeight() - 1);; // Reset clipping
        return mp_const_false;
    } else if (n_args == 5) {
        int x1 = mp_obj_get_int(args[1]);
        int y1 = mp_obj_get_int(args[2]);
        int x2 = mp_obj_get_int(args[3]);
        int y2 = mp_obj_get_int(args[4]);
        self->gfx->ClipWindow(x1, y1, x2, y2);
        return mp_const_true;
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("ClipWindow() or ClipWindow(x1, y1, x2, y2)"));
        return mp_const_none;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_clip_window_obj, 1, 5, mp_graphics4d_clip_window);

// Method: MoveTo(x, y)
extern "C" mp_obj_t mp_graphics4d_move_to(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    self->gfx->MoveTo(x, y);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_move_to_obj, 3, 3, mp_graphics4d_move_to);

// Method: MoveRel(x_off, y_off)
extern "C" mp_obj_t mp_graphics4d_move_rel(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x_off = mp_obj_get_int(args[1]);
    int y_off = mp_obj_get_int(args[2]);
    bool success = self->gfx->MoveRel(x_off, y_off);
    return (success ? mp_const_true : mp_const_false);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_move_rel_obj, 3, 3, mp_graphics4d_move_rel);

// Method: ClearScreen(draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_clear_screen(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    bool draw_fb = (n_args > 1) ? mp_obj_is_true(args[1]) : true;
    self->gfx->Cls(draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_clear_screen_obj, 1, 2, mp_graphics4d_clear_screen);

// Method: RectangleF(x1, y1, x2, y2, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_rectangle_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->RectangleFilled(x1, y1, x2, y2, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_rectangle_filled_obj, 6, 7, mp_graphics4d_rectangle_filled);

// Method: RectangleFAlpha(x1, y1, x2, y2, buffer, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_rectangle_filled_alpha(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[5], &bufinfo, MP_BUFFER_READ);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->RectangleFilledWithAlpha(x1, y1, x2, y2, (const uint8_t *)bufinfo.buf, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_rectangle_filled_alpha_obj, 6, 7, mp_graphics4d_rectangle_filled_alpha);

// Method: HLine(x1, x2, y, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_hline(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int x2 = mp_obj_get_int(args[2]);
    int y = mp_obj_get_int(args[3]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[4]);
    bool draw_fb = (n_args > 5) ? mp_obj_is_true(args[5]) : true;
    self->gfx->Hline(x1, x2, y, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_hline_obj, 4, 5, mp_graphics4d_hline);

// Method: VLine(x, y1, y2, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_vline(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int y2 = mp_obj_get_int(args[3]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[4]);
    bool draw_fb = (n_args > 5) ? mp_obj_is_true(args[5]) : true;
    self->gfx->Vline(x, y1, y2, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_vline_obj, 4, 5, mp_graphics4d_vline);

// Method: Rectangle(x1, y1, x2, y2, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_rectangle(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->Rectangle(x1, y1, x2, y2, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_rectangle_obj, 6, 7, mp_graphics4d_rectangle);

// Method: Pixel(x, y, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_put_pixel(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[3]);
    bool draw_fb = (n_args > 4) ? mp_obj_is_true(args[4]) : true;
    self->gfx->PutPixel(x, y, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_put_pixel_obj, 4, 5, mp_graphics4d_put_pixel);

// Method: Line(x1, y1, x2, y2, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_line(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->Line(x1, y1, x2, y2, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_line_obj, 6, 7, mp_graphics4d_line);

// Method: Ellipse(xcenter, ycenter, radx, rady, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_ellipse(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int xc = mp_obj_get_int(args[1]);
    int yc = mp_obj_get_int(args[2]);
    unsigned int radx = (unsigned int)mp_obj_get_int(args[3]);
    unsigned int rady = (unsigned int)mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->Ellipse(xc, yc, radx, rady, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_ellipse_obj, 6, 7, mp_graphics4d_ellipse);

// Method: EllipseF(xcenter, ycenter, radx, rady, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_ellipse_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int xc = mp_obj_get_int(args[1]);
    int yc = mp_obj_get_int(args[2]);
    unsigned int radx = (unsigned int)mp_obj_get_int(args[3]);
    unsigned int rady = (unsigned int)mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 6) ? mp_obj_is_true(args[6]) : true;
    self->gfx->EllipseFilled(xc, yc, radx, rady, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_ellipse_filled_obj, 6, 7, mp_graphics4d_ellipse_filled);

// Method: Circle(xc, yc, radius, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_circle(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int xc = mp_obj_get_int(args[1]);
    int yc = mp_obj_get_int(args[2]);
    unsigned int radius = (unsigned int)mp_obj_get_int(args[3]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[4]);
    bool draw_fb = (n_args > 5) ? mp_obj_is_true(args[5]) : true;
    self->gfx->Circle(xc, yc, radius, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_circle_obj, 5, 6, mp_graphics4d_circle);

// Method: Arc(xa, ya, radius, sa, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_arc(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int xa = mp_obj_get_int(args[1]);
    int ya = mp_obj_get_int(args[2]);
    unsigned int radius = (unsigned int)mp_obj_get_int(args[3]);
    int sa = mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    bool draw_fb = (n_args > 7) ? mp_obj_is_true(args[6]) : true;
    self->gfx->Arc(xa, ya, radius, sa, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_arc_obj, 6, 7, mp_graphics4d_arc);

// Method: ArcF(xa, ya, radius, sa, ea, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_arc_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int xa = mp_obj_get_int(args[1]);
    int ya = mp_obj_get_int(args[2]);
    unsigned int radius = (unsigned int)mp_obj_get_int(args[3]);
    int sa = mp_obj_get_int(args[4]);
    int ea = mp_obj_get_int(args[5]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[6]);
    bool draw_fb = (n_args > 7) ? mp_obj_is_true(args[7]) : true;
    self->gfx->ArcFilled(xa, ya, radius, sa, ea, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_arc_filled_obj, 7, 8, mp_graphics4d_arc_filled);

// Method: CircleF(xcenter, ycenter, radius, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_circle_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int xc = mp_obj_get_int(args[1]);
    int yc = mp_obj_get_int(args[2]);
    unsigned int radius = (unsigned int)mp_obj_get_int(args[3]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[4]);
    bool draw_fb = (n_args > 5) ? mp_obj_is_true(args[5]) : true;
    self->gfx->CircleFilled(xc, yc, radius, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_circle_filled_obj, 5, 6, mp_graphics4d_circle_filled);

// Method: Triangle(x1, y1, x2, y2, x3, y3, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_triangle(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    int x3 = mp_obj_get_int(args[5]);
    int y3 = mp_obj_get_int(args[6]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[7]);
    bool draw_fb = (n_args > 8) ? mp_obj_is_true(args[8]) : true;
    self->gfx->Triangle(x1, y1, x2, y2, x3, y3, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_triangle_obj, 7, 8, mp_graphics4d_triangle);

// Method: TriangleF(x1, y1, x2, y2, x3, y3, color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_triangle_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    int x3 = mp_obj_get_int(args[5]);
    int y3 = mp_obj_get_int(args[6]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[7]);
    bool draw_fb = (n_args > 8) ? mp_obj_is_true(args[8]) : true;
    self->gfx->TriangleFilled(x1, y1, x2, y2, x3, y3, color, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_triangle_filled_obj, 7, 8, mp_graphics4d_triangle_filled);

// List ->> Array converter
static void mp_obj_to_int_array(mp_obj_t list_obj, int *out_array, size_t *out_len) {
    mp_obj_t *items;
    mp_obj_get_array(list_obj, out_len, &items);
    if (*out_len > 256) {
         mp_raise_ValueError(MP_ERROR_TEXT("Polygon list too long (max 256)"));
    }
    for (size_t i = 0; i < *out_len; i++) {
        out_array[i] = mp_obj_get_int(items[i]);
    }
}
// Methon: Polyline([x1, x2, ...], [y1, y2, ...], color, draw_fb=True)
//extern "C" mp_obj_t mp_graphics4d_polyline(mp_obj_t vx_obj, mp_obj_t vy_obj, mp_obj_t color_obj, mp_obj_t draw_fb_obj) {
extern "C" mp_obj_t mp_graphics4d_polyline(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int vx[256], vy[256];
    size_t len_x, len_y;
    mp_obj_to_int_array(args[1], vx, &len_x);
    mp_obj_to_int_array(args[2], vy, &len_y);
    if (len_x != len_y) {
        mp_raise_ValueError(MP_ERROR_TEXT("x and y lists must be same length"));
    }
    else {
        uint16_t color = (uint16_t)mp_obj_get_int(args[3]);
        bool draw_fb = mp_obj_is_true(args[4]);
        self->gfx->Polyline(len_x, vx, vy, color, draw_fb);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_polyline_obj, 5, 5, mp_graphics4d_polyline);

// Method: Polygon([x1, x2, ...], [y1, y2, ...], color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_polygon(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int vx[256], vy[256];
    size_t len_x, len_y;
    mp_obj_to_int_array(args[1], vx, &len_x);
    mp_obj_to_int_array(args[2], vy, &len_y);
    if (len_x != len_y) {
        mp_raise_ValueError(MP_ERROR_TEXT("x and y lists must be same length"));
    }
    else {
        uint16_t color = (uint16_t)mp_obj_get_int(args[3]);
        bool draw_fb = mp_obj_is_true(args[4]);
        self->gfx->Polygon(len_x, vx, vy, color, draw_fb);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_polygon_obj, 5, 5, mp_graphics4d_polygon);

// Method: PolygonF([x1, x2, ...], [y1, y2, ...], color, draw_fb=True)
extern "C" mp_obj_t mp_graphics4d_polygon_filled(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int vx[256], vy[256];
    size_t len_x, len_y;
    mp_obj_to_int_array(args[1], vx, &len_x);
    mp_obj_to_int_array(args[2], vy, &len_y);
    if (len_x != len_y) {
        mp_raise_ValueError(MP_ERROR_TEXT("x and y lists must be same length"));
    }
    else {
        uint16_t color = (uint16_t)mp_obj_get_int(args[3]);
        bool draw_fb = mp_obj_is_true(args[4]);
        self->gfx->PolygonFilled(len_x, vx, vy, color, draw_fb);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_polygon_filled_obj, 5, 5, mp_graphics4d_polygon_filled);

// ~ __write_command
// ~ __write_data
// ~ __read_data
// ~ __get_aux_buffer x2

// Method: setFont(font: int)
extern "C" mp_obj_t mp_graphics4d_set_font(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int font_id = mp_obj_get_int(args[1]);
    const uint8_t *font_ptr = NULL;
    switch (font_id) {
        case 1: font_ptr = Font1; break;
        case 2: font_ptr = Font2; break;
        case 3: font_ptr = Font3; break;
        case 4: font_ptr = Font4; break;
        default: break;
    }
    const uint8_t *last_font = self->gfx->SetFont(font_ptr);
    return mp_obj_new_int_from_uint((uintptr_t)last_font);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_set_font_obj, 2, 2, mp_graphics4d_set_font);

// Method: setFontForeground(color)
extern "C" mp_obj_t mp_graphics4d_set_font_fg(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    uint16_t color = mp_obj_get_int(args[1]);
    uint16_t old_color = self->gfx->SetFontForeground(color);
    return mp_obj_new_int(old_color);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_set_font_fg_obj, 2, 2, mp_graphics4d_set_font_fg);

// Method: setFontBackground(color)
extern "C" mp_obj_t mp_graphics4d_set_font_bg(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    uint16_t color = mp_obj_get_int(args[1]);
    uint16_t old_color = self->gfx->SetFontBackground(color);
    return mp_obj_new_int(old_color);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_set_font_bg_obj, 2, 2, mp_graphics4d_set_font_bg);

// Method: getStringWidth(string: str)
extern "C" mp_obj_t mp_graphics4d_get_string_width(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    const char *ts = mp_obj_str_get_str(args[1]);
    uint width = self->gfx->GetStringWidth(ts);
    return mp_obj_new_int(width);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_get_string_width_obj, 2, 2, mp_graphics4d_get_string_width);

// Method: getFontHeight
extern "C" mp_obj_t mp_graphics4d_get_font_height(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    uint height = self->gfx->GetFontHeight();
    return mp_obj_new_int(height);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_get_font_height_obj, 1, 1, mp_graphics4d_get_font_height);

// ~ __putch


// Method: print(text, draw_fb=True) || print(textarea, text, draw_fb_True)
extern "C" mp_obj_t mp_graphics4d_print(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    // Case: print(str, draw_fb)
    if (n_args == 2 || (n_args == 3 && mp_obj_is_str(args[1]))) {
        const char *str = mp_obj_str_get_str(args[1]);
        bool draw_fb = (n_args == 3) ? mp_obj_is_true(args[2]) : true;
        size_t printed = self->gfx->print(str, draw_fb);
        return mp_obj_new_bool(printed);
    }
    // Case: print(text_area, str, draw_fb)
    else if (n_args >= 3 && mp_obj_is_type(args[1], &graphics4d_TextArea_type)) {
        mp_obj_textarea_t *ta = static_cast<mp_obj_textarea_t*>(MP_OBJ_TO_PTR(args[1]));
        const char *str = mp_obj_str_get_str(args[2]);
        bool draw_fb = (n_args == 4) ? mp_obj_is_true(args[3]) : true;
        size_t printed = self->gfx->print(ta->ta, str, draw_fb);
        return mp_obj_new_bool(printed);
    }
    else {
        mp_raise_TypeError(MP_ERROR_TEXT("print(str, [draw_fb]) or print(textarea, str, [draw_fb])"));
        return mp_const_none;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_print_obj, 2, 4, mp_graphics4d_print);

/*// Method: printf(text, *args, draw_fb=True) || printf(textarea, text, *args, draw_fb_True)
extern "C" mp_obj_t mp_graphics4d_printf(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    // Initialize dynamic String-Buffer (vstr)
    vstr_t vstr;
    mp_print_t print;
    vstr_init(&vstr, 64);
    // ALT: mp_print_strn_init(&print, vstr_str(&vstr), 0, vstr_size(&vstr));
    //mp_print_strn_init(&print, vstr.buf, 0, vstr.len);
    mp_print_strn_init_vstr(&print, &vstr);
    // Case: printf(str_format, *args)
    if (n_args >= 2 && mp_obj_is_str(args[1])) {
        mp_obj_t format_str = args[1];
        mp_obj_sprintf(&print, format_str, n_args - 2, &args[2]);
        const char *str = vstr_null_terminated_str(&vstr);
        size_t printed = self->gfx->print(str);
        vstr_clear(&vstr); // Puffer freigeben
        return mp_obj_new_int(printed);
    }
    // Case: printf(text_area, str_format, *args)
    if (n_args >= 3 && mp_obj_is_type(args[1], &graphics4d_TextArea_type)) {
        mp_obj_textarea_t *ta = static_cast<mp_obj_textarea_t*>(MP_OBJ_TO_PTR(args[1]));
        mp_obj_t format_str = args[2];
        mp_obj_sprintf(&print, format_str, n_args - 3, &args[3]);
        const char *str = vstr_null_terminated_str(&vstr);
        size_t printed = self->gfx->print(ta->ta, str); // Rufe print() auf
        vstr_clear(&vstr); // Puffer freigeben
        return mp_obj_new_int(printed);
    }
    vstr_clear(&vstr);
    mp_raise_TypeError(MP_ERROR_TEXT("printf(format, *args) or printf(textarea, format, *args)"));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(mp_graphics4d_printf_obj, 2, mp_graphics4d_printf);*/

// Method: CreateTextArea(x1, y1, x2, y2, fg_color, bg_color)
extern "C" mp_obj_t mp_graphics4d_create_text_area(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    uint16_t fg = mp_obj_get_int(args[5]);
    uint16_t bg = mp_obj_get_int(args[6]);
    TextArea4D area = self->gfx->CreateTextArea(x1, y1, x2, y2, fg, bg);
    mp_obj_textarea_t *obj = m_new_obj(mp_obj_textarea_t);
    obj->base.type = &graphics4d_TextArea_type;
    obj->ta = area;
    return MP_OBJ_FROM_PTR(obj);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_create_text_area_obj, 7, 7, mp_graphics4d_create_text_area);

//###
/* Class: ImageControl - 2358 (?)
// Method: ImageControl
// Method: ~ImageControl
 */

/*
GRAPHICSMEDIA4D
// überladene Funktionen    einen Wrapper schreiben, der zur Laufzeit entscheidet, welche C++-Überladung aufzurufen ist
*/

// Method: LoadImageControl(filename, x,y) || LoadImageControl(gci_array)
// Factory-Funktion: graphics4d.LoadImageControl(filename, [formIndex]) ODER graphics4d.LoadImageControl(buffer, [formIndex])
extern "C" mp_obj_t mp_graphics4d_load_image_control(size_t n_args, const mp_obj_t *args) {
    if (n_args < 1 || n_args > 2) {mp_raise_TypeError(MP_ERROR_TEXT("LoadImageControl(file_or_buf, [formIndex])"));}
    uint formIndex = (n_args == 2) ? mp_obj_get_int(args[1]) : -1;
    ImageControl4D hndl = NULL;
    if (mp_obj_is_str(args[0])) {
        // Overload 1: LoadImageControl(filename, [formIndex])
        const char *filename = mp_obj_str_get_str(args[0]);
        //hndl = img.LoadImageControl(filename, formIndex); // C++ call
        hndl = GraphicsMedia4D::GetInstance().LoadImageControl(filename, formIndex); // C++ call
    } else {
        // Overload 2: LoadImageControl(buffer, [formIndex])
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[0], &bufinfo, MP_BUFFER_READ);
        //hndl = img.LoadImageControl((const uint8_t *)bufinfo.buf, formIndex); // C++ call
        hndl = GraphicsMedia4D::GetInstance().LoadImageControl((const uint8_t *)bufinfo.buf, formIndex); // C++ call
    }
    if (!hndl) {mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to load image control"));}
    mp_obj_imagecontrol_t *o = m_new_obj(mp_obj_imagecontrol_t);
    o->base.type = &graphics4d_ImageControl_type;
    o->hndl = hndl;
    return MP_OBJ_FROM_PTR(o);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_graphics4d_load_image_control_obj, 1, 2, mp_graphics4d_load_image_control);

// Method: ImageControl.getCount()
extern "C" mp_obj_t mp_imagecontrol_get_count(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = (mp_obj_imagecontrol_t *)MP_OBJ_TO_PTR(args[0]);
    if (!self->hndl) mp_raise_ValueError(MP_ERROR_TEXT("ImageControl is closed"));
    //int count = img.GetCount(self->hndl);
    int count = GraphicsMedia4D::GetInstance().GetCount(self->hndl);
    return mp_obj_new_int(count);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_get_count_obj, 1, 1, mp_imagecontrol_get_count);

// Method: ImageControl.getInfo(index)
extern "C" mp_obj_t mp_imagecontrol_get_info(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = (mp_obj_imagecontrol_t *)MP_OBJ_TO_PTR(args[0]);
    if (!self->hndl) mp_raise_ValueError(MP_ERROR_TEXT("ImageControl is closed"));
    int index = mp_obj_get_int(args[1]);
    //MediaInfo4D info = img.GetInfo(self->hndl, index); 
    MediaInfo4D info = GraphicsMedia4D::GetInstance().GetInfo(self->hndl, index);
    // HINWEIS: Das gibt einen Zeiger zurück, vielleicht in ein Tupel/Dict umwandeln?
    // Fürs Erste Zeiger als int
    return mp_obj_new_int_from_uint((uintptr_t)info);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_get_info_obj, 2, 2, mp_imagecontrol_get_info);

// Method: ImageControl.setValue(index, value)
extern "C" mp_obj_t mp_imagecontrol_set_value(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = (mp_obj_imagecontrol_t *)MP_OBJ_TO_PTR(args[0]);
    if (!self->hndl) mp_raise_ValueError(MP_ERROR_TEXT("ImageControl is closed"));
    int index = mp_obj_get_int(args[1]);
    int value = mp_obj_get_int(args[2]);
    //img.SetValue(self->hndl, index, value);
    GraphicsMedia4D::GetInstance().SetValue(self->hndl, index, value);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_set_value_obj, 3, 3, mp_imagecontrol_set_value);

// Method: ImageControl.getValue(index)
extern "C" mp_obj_t mp_imagecontrol_get_value(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = (mp_obj_imagecontrol_t *)MP_OBJ_TO_PTR(args[0]);
    if (!self->hndl) mp_raise_ValueError(MP_ERROR_TEXT("ImageControl is closed"));
    int index = mp_obj_get_int(args[1]);
    //int value = img.GetValue(self->hndl, index);
    int value = GraphicsMedia4D::GetInstance().GetValue(self->hndl, index);
    return mp_obj_new_int(value);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_get_value_obj, 2, 2, mp_imagecontrol_get_value);

// Method: ImageControl.show(index, [draw_fb])
extern "C" mp_obj_t mp_imagecontrol_show(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = static_cast<mp_obj_imagecontrol_t*>(MP_OBJ_TO_PTR(args[0]));
    if (!self->hndl) mp_raise_ValueError(MP_ERROR_TEXT("ImageControl is closed"));
    int index = mp_obj_get_int(args[1]);
    bool draw_fb = (n_args > 2) ? mp_obj_is_true(args[2]) : true;
    //img.Show(self->hndl, index, draw_fb);
    GraphicsMedia4D::GetInstance().Show(self->hndl, index, draw_fb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_show_obj, 2, 3, mp_imagecontrol_show);

// Method: ImageControl.showForm(index)
extern "C" mp_obj_t mp_imagecontrol_show_form(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = (mp_obj_imagecontrol_t *)MP_OBJ_TO_PTR(args[0]);
    if (!self->hndl) mp_raise_ValueError(MP_ERROR_TEXT("ImageControl is closed"));
    int index = mp_obj_get_int(args[1]);
    //img.ShowForm(self->hndl, index);
    GraphicsMedia4D::GetInstance().ShowForm(self->hndl, index);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_show_form_obj, 2, 2, mp_imagecontrol_show_form);

// Method: ImageControl.touched(index)
extern "C" mp_obj_t mp_imagecontrol_touched(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = (mp_obj_imagecontrol_t *)MP_OBJ_TO_PTR(args[0]);
    if (!self->hndl) mp_raise_ValueError(MP_ERROR_TEXT("ImageControl is closed"));
    int index = mp_obj_get_int(args[1]);
    //uint result = img.Touched(self->hndl, index);
    uint result = GraphicsMedia4D::GetInstance().Touched(self->hndl, index);
    return mp_obj_new_int(result);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_touched_obj, 2, 2, mp_imagecontrol_touched);


// ~ __draw_to_buffer
// ~ __show_digits
// ~ __show_2_frame_gauge
// ~ __show_linear_gauge
// ~ __show_knob
// ~ __redraw_form_region


// Define locals(methods)-table for the Graphics4D Python object
static const mp_rom_map_elem_t mp_graphics4d_locals_dict_table[] = {
    //{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_graphics4d_make_new_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mp_graphics4d_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mp_graphics4d_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_DrawWidget), MP_ROM_PTR(&mp_graphics4d_draw_widget_obj)},
    { MP_ROM_QSTR(MP_QSTR_BlendColor), MP_ROM_PTR(&mp_graphics4d_blend_color_obj)},
    { MP_ROM_QSTR(MP_QSTR_getWidth), MP_ROM_PTR(&mp_graphics4d_get_width_obj)},
    { MP_ROM_QSTR(MP_QSTR_getHeight), MP_ROM_PTR(&mp_graphics4d_get_height_obj)},
    { MP_ROM_QSTR(MP_QSTR_setBacklightLevel), MP_ROM_PTR(&mp_graphics4d_set_backlight_level_obj)},
    { MP_ROM_QSTR(MP_QSTR_setContrast), MP_ROM_PTR(&mp_graphics4d_contrast_obj)},
    { MP_ROM_QSTR(MP_QSTR_Screenmode), MP_ROM_PTR(&mp_graphics4d_screenmode_obj)},
    { MP_ROM_QSTR(MP_QSTR_setBackgroundColor), MP_ROM_PTR(&mp_graphics4d_set_background_color_obj)},
    { MP_ROM_QSTR(MP_QSTR_setClipWindow), MP_ROM_PTR(&mp_graphics4d_clip_window_obj)},
    { MP_ROM_QSTR(MP_QSTR_MoveTo), MP_ROM_PTR(&mp_graphics4d_move_to_obj)},
    { MP_ROM_QSTR(MP_QSTR_MoveRel), MP_ROM_PTR(&mp_graphics4d_move_rel_obj)},
    { MP_ROM_QSTR(MP_QSTR_ClearScreen), MP_ROM_PTR(&mp_graphics4d_clear_screen_obj)},
    { MP_ROM_QSTR(MP_QSTR_Rectangle), MP_ROM_PTR(&mp_graphics4d_rectangle_obj)},
    { MP_ROM_QSTR(MP_QSTR_RectangleF), MP_ROM_PTR(&mp_graphics4d_rectangle_filled_obj)},
    { MP_ROM_QSTR(MP_QSTR_RectangleFAlpha), MP_ROM_PTR(&mp_graphics4d_rectangle_filled_alpha_obj)},
    { MP_ROM_QSTR(MP_QSTR_Circle), MP_ROM_PTR(&mp_graphics4d_circle_obj)},
    { MP_ROM_QSTR(MP_QSTR_CircleF), MP_ROM_PTR(&mp_graphics4d_circle_filled_obj)},
    { MP_ROM_QSTR(MP_QSTR_Ellipse), MP_ROM_PTR(&mp_graphics4d_ellipse_obj)},
    { MP_ROM_QSTR(MP_QSTR_EllipseF), MP_ROM_PTR(&mp_graphics4d_ellipse_filled_obj)},
    { MP_ROM_QSTR(MP_QSTR_Line), MP_ROM_PTR(&mp_graphics4d_line_obj)},
    { MP_ROM_QSTR(MP_QSTR_HLine), MP_ROM_PTR(&mp_graphics4d_hline_obj)},
    { MP_ROM_QSTR(MP_QSTR_VLine), MP_ROM_PTR(&mp_graphics4d_vline_obj)},
    { MP_ROM_QSTR(MP_QSTR_Pixel), MP_ROM_PTR(&mp_graphics4d_put_pixel_obj)},
    { MP_ROM_QSTR(MP_QSTR_Triangle), MP_ROM_PTR(&mp_graphics4d_triangle_obj)},
    { MP_ROM_QSTR(MP_QSTR_TriangleF), MP_ROM_PTR(&mp_graphics4d_triangle_filled_obj)},
    { MP_ROM_QSTR(MP_QSTR_Polyline), MP_ROM_PTR(&mp_graphics4d_polyline_obj)},
    { MP_ROM_QSTR(MP_QSTR_Polygon), MP_ROM_PTR(&mp_graphics4d_polygon_obj)},
    { MP_ROM_QSTR(MP_QSTR_PolygonF), MP_ROM_PTR(&mp_graphics4d_polygon_filled_obj)},
    { MP_ROM_QSTR(MP_QSTR_setFont), MP_ROM_PTR(&mp_graphics4d_set_font_obj)},
    { MP_ROM_QSTR(MP_QSTR_setFontForeground), MP_ROM_PTR(&mp_graphics4d_set_font_fg_obj)},
    { MP_ROM_QSTR(MP_QSTR_setFontBackground), MP_ROM_PTR(&mp_graphics4d_set_font_bg_obj)},
    { MP_ROM_QSTR(MP_QSTR_getStringWidth), MP_ROM_PTR(&mp_graphics4d_get_string_width_obj)},
    { MP_ROM_QSTR(MP_QSTR_getFontHeight), MP_ROM_PTR(&mp_graphics4d_get_font_height_obj)},
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&mp_graphics4d_print_obj)},
    //{ MP_ROM_QSTR(MP_QSTR_printf), MP_ROM_PTR(&mp_graphics4d_printf_obj)},
    { MP_ROM_QSTR(MP_QSTR_createTextArea), MP_ROM_PTR(&mp_graphics4d_create_text_area_obj)},
    
};
static MP_DEFINE_CONST_DICT(mp_graphics4d_locals_dict, mp_graphics4d_locals_dict_table);

// Define the Python type object for Graphics4D
MP_DEFINE_CONST_OBJ_TYPE(
    graphics4d_Graphics4D_type,
    MP_QSTR_Graphics4D,
    MP_TYPE_FLAG_NONE,
    make_new, (const void*)mp_graphics4d_make_new,
    locals_dict, &mp_graphics4d_locals_dict
);//*/
/*const mp_obj_type_t graphics4d_Graphics4D_type = {
    .base = { &mp_type_type },
    .name = MP_QSTR_Graphics4D,
    .make_new = mp_graphics4d_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_graphics4d_locals_dict,
};//*/

// Define the module's globals table
static const mp_rom_map_elem_t mp_graphics4d_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_graphics4d) },
    
    // Classes
    { MP_ROM_QSTR(MP_QSTR_Graphics4D), MP_ROM_PTR(&graphics4d_Graphics4D_type) },
    { MP_ROM_QSTR(MP_QSTR_Touch), MP_ROM_PTR(&graphics4d_Touch4D_type) },
    { MP_ROM_QSTR(MP_QSTR_TextArea), MP_ROM_PTR(&graphics4d_TextArea_type) },
    { MP_ROM_QSTR(MP_QSTR_ImageControl), MP_ROM_PTR(&graphics4d_ImageControl_type) },
    { MP_ROM_QSTR(MP_QSTR_LoadImageControl), MP_ROM_PTR(&mp_graphics4d_load_image_control_obj) },///???

    // Constants for Touch Status
    { MP_ROM_QSTR(MP_QSTR_NO_TOUCH), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_TOUCH_PRESSED), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_TOUCH_RELEASED), MP_ROM_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_TOUCH_MOVING), MP_ROM_INT(3) },
};
static MP_DEFINE_CONST_DICT(mp_graphics4d_module_globals, mp_graphics4d_module_globals_table);

// Define the module object
const mp_obj_module_t graphics4d_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_graphics4d_module_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(graphics4d, graphics4d_user_cmodule);


/*
GRAPHICSTOUCH4D
*/
// Define the Python object structure for Touch
typedef struct _mp_obj_touch4d_t {
    mp_obj_base_t base;
    GraphicsTouch4D *touch; // Pointer to the C++ GraphicsTouch4D singleton instance
} mp_obj_touch4d_t;


// Constructor for the Python Touch class: touch = graphics4d.Touch4D()
extern "C" mp_obj_t mp_touch4d_make_new(const mp_obj_type_t *type,
                                     size_t n_args, size_t n_kw,
                                     const mp_obj_t *args) {
    // No arguments are expected for the constructor
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    // Allocate memory for the Python object
    mp_obj_touch4d_t *self = m_new_obj(mp_obj_touch4d_t);
    self->base.type = (mp_obj_type_t *)type;
    // Get the singleton instance of the C++ GraphicsTouch4D class
    self->touch = &GraphicsTouch4D::GetInstance();
    // Initialize the touch controller
    bool ok = self->touch->Initialize();
    if (!ok) {
        self->touch = NULL; 
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize GraphicsTouch4D"));
    }
    return MP_OBJ_FROM_PTR(self);
}
/*static MP_DEFINE_CONST_FUN_OBJ_KW(mp_touch4d_make_new_obj, 0, mp_touch4d_make_new);*/

// Destructor / deinit method
extern "C" mp_obj_t mp_touch4d_deinit(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    if (self->touch) {
        //delete self->touch;
        (void)self;
        //self->touch = NULL; 
        self->touch = nullptr;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_deinit_obj, 1, 1, mp_touch4d_deinit);
// close()-alias for deinit
extern "C" mp_obj_t mp_touch4d_close(size_t n_args, const mp_obj_t *args) {
    return mp_touch4d_deinit(1, args);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_close_obj, 1, 1, mp_touch4d_close);

// Convenience macro to check if the touch object is initialized
#define TOUCH_CHECK(self)     if (!self || !(self)->touch) {mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("GraphicsTouch4D not initialized"));}

/*// Method: Calibrate() -> bool
extern "C" mp_obj_t mp_touch4d_calibrate(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    bool result = self->touch->Calibrate();
    return mp_obj_new_bool(result);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_calibrate_obj, 1, 1, mp_touch4d_calibrate);*/

// ~ __set_calibration
// ~ __get_calibration

// Method: getPoints() -> int
extern "C" mp_obj_t mp_touch4d_get_points(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t points = self->touch->GetPoints();
    return mp_obj_new_int(points);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_get_points_obj, 1, 1, mp_touch4d_get_points);

// Method: getStatus(point=0) -> int
extern "C" mp_obj_t mp_touch4d_get_status(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int8_t status = self->touch->GetStatus(point);
    return mp_obj_new_int(status);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_get_status_obj, 1, 2, mp_touch4d_get_status);

// Method: getID(point=0) -> int
extern "C" mp_obj_t mp_touch4d_get_id(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t id = self->touch->GetID(point);
    return mp_obj_new_int(id);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_get_id_obj, 1, 2, mp_touch4d_get_id);

// Method: getX(point=0) -> int
extern "C" mp_obj_t mp_touch4d_get_x(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t x = self->touch->GetX(point);
    return mp_obj_new_int(x);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_get_x_obj, 1, 2, mp_touch4d_get_x);

// Method: getY(point=0) -> int
extern "C" mp_obj_t mp_touch4d_get_y(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t y = self->touch->GetY(point);
    return mp_obj_new_int(y);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_get_y_obj, 1, 2, mp_touch4d_get_y);

// ~ __get_raw_x
// ~ __get_raw_y

// Method: getWeight(point=0) -> int
extern "C" mp_obj_t mp_touch4d_get_weight(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t weight = self->touch->GetWeight(point);
    return mp_obj_new_int(weight);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_get_weight_obj, 1, 2, mp_touch4d_get_weight);

// Method: getArea(point=0) -> int
extern "C" mp_obj_t mp_touch4d_get_area(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t area = self->touch->GetArea(point);
    return mp_obj_new_int(area);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_touch4d_get_area_obj, 1, 2, mp_touch4d_get_area);


// Define locals(methods)-table for the Touch Python object
static const mp_rom_map_elem_t mp_touch_locals_dict_table[] = {
    //{ MP_ROM_QSTR(MP_QSTR_Touch4D), MP_ROM_PTR(&mp_touch4d_make_new_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mp_touch4d_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mp_touch4d_close_obj) },
    //{ MP_ROM_QSTR(MP_QSTR_Calibrate), MP_ROM_PTR(&mp_touch4d_calibrate_obj) },
    { MP_ROM_QSTR(MP_QSTR_getPoints), MP_ROM_PTR(&mp_touch4d_get_points_obj) },
    { MP_ROM_QSTR(MP_QSTR_getStatus), MP_ROM_PTR(&mp_touch4d_get_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_getID), MP_ROM_PTR(&mp_touch4d_get_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_getX), MP_ROM_PTR(&mp_touch4d_get_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_getY), MP_ROM_PTR(&mp_touch4d_get_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_getWeight), MP_ROM_PTR(&mp_touch4d_get_weight_obj) },
    { MP_ROM_QSTR(MP_QSTR_getArea), MP_ROM_PTR(&mp_touch4d_get_area_obj) },
};
static MP_DEFINE_CONST_DICT(mp_touch_locals_dict, mp_touch_locals_dict_table);

// Define the Python type object for Graphics4D Touch
MP_DEFINE_CONST_OBJ_TYPE(
    graphics4d_Touch4D_type,
    MP_QSTR_Touch,
    MP_TYPE_FLAG_NONE,
    make_new, (const void*)mp_touch4d_make_new,
    locals_dict, &mp_touch_locals_dict
);//*/
/*const mp_obj_type_t graphics4d_Touch4D_type = {
    .base = { &mp_type_type },
    .name = MP_QSTR_Touch4D,
    .make_new = mp_touch4d_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_touch_locals_dict,
};//*/

//###
// deinit / close method für ImageControl
extern "C" mp_obj_t mp_imagecontrol_deinit(size_t n_args, const mp_obj_t *args) {
    mp_obj_imagecontrol_t *self = static_cast<mp_obj_imagecontrol_t*>(MP_OBJ_TO_PTR(args[0]));
    if (self->hndl) {
        // HINWEIS: Du musst ~ImageControl() in Graphics4D.h public machen!
        delete self->hndl;
        self->hndl = nullptr;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_imagecontrol_deinit_obj, 1, 1, mp_imagecontrol_deinit);

// Define locals(methods)-table for the ImageControl Python object
static const mp_rom_map_elem_t mp_imagecontrol_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mp_imagecontrol_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_getCount), MP_ROM_PTR(&mp_imagecontrol_get_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_getInfo), MP_ROM_PTR(&mp_imagecontrol_get_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_setValue), MP_ROM_PTR(&mp_imagecontrol_set_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_getValue), MP_ROM_PTR(&mp_imagecontrol_get_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_Show), MP_ROM_PTR(&mp_imagecontrol_show_obj) },
    { MP_ROM_QSTR(MP_QSTR_ShowForm), MP_ROM_PTR(&mp_imagecontrol_show_form_obj) },
    { MP_ROM_QSTR(MP_QSTR_Touched), MP_ROM_PTR(&mp_imagecontrol_touched_obj) },
};
static MP_DEFINE_CONST_DICT(mp_imagecontrol_locals, mp_imagecontrol_locals_table);

//} // close extern "C"