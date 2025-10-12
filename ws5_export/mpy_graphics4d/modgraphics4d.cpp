extern "C" {
#include "py/obj.h"
#include "py/runtime.h"
//#include "py/builtin.h"
}
#include "Graphics4D.h"

/*
TEXTAREA
*/
typedef struct _mp_obj_textarea_t {
    mp_obj_base_t base;
    TextArea *ta; // Zeiger auf die native C++-Instanz
} mp_obj_textarea_t;

// Factory-Funktion für TextArea
STATIC mp_obj_t textarea_make_new(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(textarea_make_new_obj, 6, 6, textarea_make_new);

// Zerstörung / Freigabe
STATIC mp_obj_t textarea_deinit(mp_obj_t self_in) {
    mp_obj_textarea_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->ta) {
        delete self->ta;
        self->ta = nullptr;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(textarea_deinit_obj, textarea_deinit);

// Methoden-Tabelle (nur close)
STATIC const mp_rom_map_elem_t textarea_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&textarea_deinit_obj) },
};
STATIC MP_DEFINE_CONST_DICT(textarea_locals, textarea_locals_table);

// Typdefinition
const mp_obj_type_t graphics4d_TextArea_type = {
    { &mp_type_type },
    .name = MP_QSTR_TextArea,
    .make_new = NULL, // Konstruktion nur über Factory
    .locals_dict = (mp_obj_dict_t*)&textarea_locals,
};



/*
GRAPHICS4D
*/
typedef struct _mp_obj_graphics4d_t {
    mp_obj_base_t base;
    Graphics4D *gfx; // Pointer to the C++ Graphics4D object
} mp_obj_graphics4d_t;

// Constructor for the Graphics4D class - graphics4d.init()
STATIC mp_obj_t graphics4d_make_new(const mp_obj_type_t *type,
                                     size_t n_args, size_t n_kw,
                                     const mp_obj_t *args) {
    // No arguments are expected for Constructor
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    // Create a new instance of the Graphics4D object
    mp_obj_graphics4d_t *self = m_new_obj(mp_obj_graphics4d_t); // Allocate memory for the object
    self->base.type = (mp_obj_type_t *)type;
    Graphics4D *native = new (std::nothrow) Graphics4D();
    // Secure-check Memory allocation
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
// close()-alias for deinit
STATIC mp_obj_t graphics4d_close(mp_obj_t self_in) {
    return graphics4d_deinit(self_in);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_close_obj, graphics4d_close);

// convenience guard macro for wrappers
#define GFX_CHECK(self)     if (!self || !(self)->gfx) {mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Graphics4D not initialized or closed properly"));}

// Method: DrawWidget(num, f, x, y, gci_array)
STATIC const mp_obj_t graphics4d_draw_widget(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    // Check for the correct number of arguments: 5 user args + 1 self = 6 total
    if (n_args != 6) {
        mp_raise_TypeError(MP_ERROR_TEXT("draw_widget(num, f, x, y, gci_array)"));
    }
    // Extract integer arguments (args[1] to args[4])
    int num = mp_obj_get_int(args[1]); // num - Arg 1
    int f = mp_obj_get_int(args[2]);   // f - Arg 2
    int x = mp_obj_get_int(args[3]);   // x - Arg 3
    int y = mp_obj_get_int(args[4]);   // y - Arg 4
    // Extract the buffer argument (gci_array) at args[5]
    mp_buffer_info bufinfo;
    mp_get_buffer_raise(args[5], &bufinfo, MP_BUFFER_READ);
    // Call the underlying C++ function
    self->gfx->DrawWidget(num, f, x, y, (const uint8_t *)bufinfo.buf);    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_6(graphics4d_draw_widget_obj, graphics4d_draw_widget);

// ~ SetFrameBuffer
// ~ Initialize
// ~ Reset

// Method: BlendColor(base_color, new_color, alpha)
STATIC mp_obj_t graphics4d_blend_color(mp_obj_t self_in, mp_obj_t base_color_obj, 
                                    mp_obj_t new_color_obj, mp_obj_t alpha_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    uint16_t base_color = (uint16_t)mp_obj_get_int(base_color_obj);
    uint16_t new_color = (uint16_t)mp_obj_get_int(new_color_obj);
    uint8_t alpha = (uint8_t)mp_obj_get_int(alpha_obj);
    uint16_t result_color = self->gfx->BlendColor(base_color, new_color, alpha);
    return mp_obj_new_int(result_color);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_4(graphics4d_blend_color_obj, graphics4d_blend_color);


// Method: getWidth()
STATIC mp_obj_t graphics4d_get_width(mp_obj_t self_in) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    uint width = self->gfx->GetWidth();
    return mp_obj_new_int(width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_get_width_obj, graphics4d_get_width);

// Method: getHeight()
STATIC mp_obj_t graphics4d_get_height(mp_obj_t self_in) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    uint height = self->gfx->GetHeight();
    return mp_obj_new_int(height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_get_height_obj, graphics4d_get_height);

// Method: setBacklightLevel(int)
STATIC mp_obj_t graphics4d_set_backlight_level(mp_obj_t self_in, mp_obj_t level_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int level = mp_obj_get_int(level_obj);
    self->gfx->SetBacklightLevel(level);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_set_backlight_level_obj, graphics4d_set_backlight_level);

// Method: contrast(int)
STATIC mp_obj_t graphics4d_contrast(mp_obj_t self_in, mp_obj_t contrast_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int contrast = mp_obj_get_int(contrast_obj);
    self->gfx->Contrast(contrast);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_contrast_obj, graphics4d_contrast);

// Method: Screenmode(orientation)
STATIC mp_obj_t graphics4d_screenmode(mp_obj_t self_in, mp_obj_t orientation_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int mode = mp_obj_get_int(orientation_obj);
    self->gfx->ScreenMode(mode);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_screenmode_obj, graphics4d_screenmode);

// ~ SetAddressWindow
// ~ SendFrameBuffer
// ~ GetFrameBuffer

// Method: setBackgroundColor(new_color)
STATIC mp_obj_t graphics4d_set_background_color(mp_obj_t self_in, mp_obj_t color_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    uint16_t color = (uint16_t)mp_obj_get_int(color_obj);
    uint16_t old_color = self->gfx->SetBackgroundColor(color);
    return mp_obj_new_int(old_color);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_set_background_color_obj, graphics4d_set_background_color);

// Method: ClipWindow(x1, y1, x2, y2) || ClipWindow() -> reset
STATIC mp_obj_t graphics4d_clip_window(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    if (n_args == 1) {
        self->gfx->ClipWindow(); // Reset clipping
        return mo_const_bool(false);
    } else if (n_args == 5) {
        int x1 = mp_obj_get_int(args[1]);
        int y1 = mp_obj_get_int(args[2]);
        int x2 = mp_obj_get_int(args[3]);
        int y2 = mp_obj_get_int(args[4]);
        self->gfx->ClipWindow(x1, y1, x2, y2);
        return mo_const_bool(true);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("ClipWindow() or ClipWindow(x1, y1, x2, y2)"));
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_clip_window_obj, 1, 5, graphics4d_clip_window);

// Method: MoveTo(x, y)
STATIC mp_obj_t graphics4d_move_to(mp_obj_t self_in, mp_obj_t x_obj, mp_obj_t y_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int x = mp_obj_get_int(x_obj);
    int y = mp_obj_get_int(y_obj);
    self->gfx->MoveTo(x, y);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(graphics4d_move_to_obj, graphics4d_move_to);

// Method: MoveRel(x_off, y_off)
STATIC mp_obj_t graphics4d_move_rel(mp_obj_t self_in, mp_obj_t x_off_obj, mp_obj_t y_off_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int x_off = mp_obj_get_int(x_off_obj);
    int y_off = mp_obj_get_int(y_off_obj);
    success = self->gfx->MoveRel(x_off, y_off);
    return mp_const_bool(success);
}

// Method: ClearScreen(draw_fb=True)
STATIC mp_obj_t graphics4d_clear_screen(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    bool draw_fb = (n_args > 1) ? mp_obj_is_true(args[1]) : true;
    self->gfx->Cls(draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_cls_obj, 1, 2, graphics4d_cls);

// Method: RectangleF(x1, y1, x2, y2, color, draw_fb=True)
STATIC mp_obj_t graphics4d_rectangle_filled(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_rectangle_filled_obj, 6, 7, graphics4d_rectangle_filled);

// Method: RectangleFAlpha(x1, y1, x2, y2, color, alpha, draw_fb=True)
STATIC mp_obj_t graphics4d_rectangle_filled_with_alpha(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[5]);
    uint8_t alpha = (uint8_t)mp_obj_get_int(args[6]);
    bool draw_fb = (n_args > 7) ? mp_obj_is_true(args[7]) : true;
    self->gfx->RectangleFilledWithAlpha(x1, y1, x2, y2, color, alpha, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_rectangle_filled_with_alpha_obj, 7, 8, graphics4d_rectangle_filled_with_alpha);

// Method: HLine(x1, x2, y, color, draw_fb=True)
STATIC mp_obj_t graphics4d_hline(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_hline_obj, 4, 5, graphics4d_hline);

// Method: VLine(x, y1, y2, color, draw_fb=True)
STATIC mp_obj_t graphics4d_vline(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_vline_obj, 4, 5, graphics4d_vline);

// Method: Rectangle(x1, y1, x2, y2, color, draw_fb=True)
STATIC mp_obj_t graphics4d_rectangle(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_rectangle_obj, 6, 7, graphics4d_rectangle);

// Method: Pixel(x, y, color, draw_fb=True)
STATIC mp_obj_t graphics4d_put_pixel(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    uint16_t color = (uint16_t)mp_obj_get_int(args[3]);
    bool draw_fb = (n_args > 4) ? mp_obj_is_true(args[4]) : true;
    self->gfx->PutPixel(x, y, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_put_pixel_obj, 4, 5, graphics4d_put_pixel);

// Method: Line(x1, y1, x2, y2, color, draw_fb=True)
STATIC mp_obj_t graphics4d_line(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_line_obj, 6, 7, graphics4d_line);

// Method: Ellipse(xcenter, ycenter, radx, rady, color, draw_fb=True)
STATIC mp_obj_t graphics4d_ellipse(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_ellipse_obj, 6, 7, graphics4d_ellipse);

// Method: EllipseF(xcenter, ycenter, radx, rady, color, draw_fb=True)
STATIC mp_obj_t graphics4d_ellipse_filled(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_ellipse_filled_obj, 6, 7, graphics4d_ellipse_filled);

// Method: Circle(xc, yc, radius, color, draw_fb=True)
STATIC mp_obj_t graphics4d_circle(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_circle_filled_obj, 5, 6, graphics4d_circle_filled);

// Method: Arc(xa, ya, radius, sa, color, draw_fb=True)
STATIC mp_obj_t graphics4d_arc(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_arc_obj, 6, 7, graphics4d_arc);

// Method: ArcF(xa, ya, radius, sa, ea, color, draw_fb=True)
STATIC mp_obj_t graphics4d_arc_filled(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_arc_filled_obj, 7, 8, graphics4d_arc_filled);

// Method: CircleF(xcenter, ycenter, radius, color, draw_fb=True)
STATIC mp_obj_t graphics4d_circle_filled(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_circle_filled_obj, 5, 6, graphics4d_circle_filled);

// Method: Triangle(x1, y1, x2, y2, x3, y3, color, draw_fb=True)
STATIC mp_obj_t graphics4d_triangle(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_triangle_obj, 7, 8, graphics4d_triangle);

// Method: TriangleF(x1, y1, x2, y2, x3, y3, color, draw_fb=True)
STATIC mp_obj_t graphics4d_triangle_filled(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_triangle_filled_obj, 7, 8, graphics4d_triangle_filled);

// List ->> Array converter
STATIC void mp_obj_to_int_array(mp_obj_t list_obj, int *out_array) {
    size_t len = mp_obj_get_array_length(list_obj);
    mp_obj_t *items;
    mp_obj_get_array(list_obj, &len, &items);
    for (size_t i = 0; i < len; i++) {
        out_array[i] = mp_obj_get_int(items[i]);
    }
}
// Methon: Polyline([x1, x2, ...], [y1, y2, ...], color, draw_fb=True)
STATIC mp_obj_t graphics4d_polyline(mp_obj_t vx_obj, mp_obj_t vy_obj, mp_obj_t color_obj, mp_obj_t draw_fb_obj) {
    GFX_CHECK(self);
    mp_obj_to_int_array(vx_obj, vx);
    mp_obj_to_int_array(vy_obj, vy);
    uint16_t color = (uint16_t)mp_obj_get_int(color_obj);
    bool draw_fb = mp_obj_is_true(draw_fb_obj);
    self->gfx->Polyline(vx, vy, len, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_4(graphics4d_polyline_obj, graphics4d_polyline);

// Method: Polygon([x1, x2, ...], [y1, y2, ...], color, draw_fb=True)
STATIC mp_obj_t graphics4d_polygon(mp_obj_t vx_obj, mp_obj_t vy_obj, mp_obj_t color_obj, mp_obj_t draw_fb_obj) {
    GFX_CHECK(self);
    mp_obj_to_int_array(vx_obj, vx);
    mp_obj_to_int_array(vy_obj, vy);
    uint16_t color = (uint16_t)mp_obj_get_int(color_obj);
    bool draw_fb = mp_obj_is_true(draw_fb_obj);
    self->gfx->Polygon(vx, vy, len, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_4(graphics4d_polygon_obj, graphics4d_polygon);

// Method: PolygonF([x1, x2, ...], [y1, y2, ...], color, draw_fb=True)
STATIC mp_obj_t graphics4d_polygon_filled(mp_obj_t vx_obj, mp_obj_t vy_obj, mp_obj_t color_obj, mp_obj_t draw_fb_obj) {
    GFX_CHECK(self);
    mp_obj_to_int_array(vx_obj, vx);
    mp_obj_to_int_array(vy_obj, vy);
    uint16_t color = (uint16_t)mp_obj_get_int(color_obj);
    bool draw_fb = mp_obj_is_true(draw_fb_obj);
    self->gfx->PolygonFilled(vx, vy, len, color, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_4(graphics4d_polygon_filled_obj, graphics4d_polygon_filled);

// ~ __write_command
// ~ __write_data
// ~ __read_data
// ~ __get_aux_buffer x2

// Method: setFont()
STATIC mp_obj_t graphics4d_setfont(size_t n_args, const mp_obj_t *args){
    GFX_CHECK(self);

    last_font = self->gfx->SetFont()
    return mp_const_int(last_font)
}

// Method: setFontForeground
// Method: setFontBackground
// Method: setFontForeground?
// Method: setFontBackground?
// Method: getStringWidth
// Method: getFontHeight
// ~ __putch


// Method: print(text, draw_fb=True)
STATIC mp_obj_t graphics4d_print(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    const char *str = mp_obj_str_get_str(args[1]);
    bool draw_fb = (n_args > 2) ? mp_obj_is_true(args[2]) : true;
    self->gfx->print(str, draw_fb);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_print_obj, 2, 3, graphics4d_print);

// Method: printf
// Method: CreateTextArea
// Method: print (TA)
// Method: printf (TA)

/* Class: ImageControl - 2358 (?)

 * Class: GraphicsMedia4D - 2597
// Method: LoadImageControl          (zweimal überladen)    einen Wrapper schreiben, der zur Laufzeit entscheidet, welche C++-Überladung aufzurufen ist
// Method: getCount
// Method: getInfo
// Method: setProperties
// Method: setValue
// Method: getValue
// Method: getFrames
// Method: setPosition
// Method: Clear
// Method: Show
// Method: ShowForm
// Method: Touched
// Method: __draw_to_buffer          (zweimal / überladen)
// Method: __show_digits
// Method: __show_2_frame_gauge
// Method: __show_linear_gauge
// Method: __show_knob
// Method: __redraw_form_region

 * Class: GraphicsTouch4D - 4326
// Method: Initialize
// Method: Calibrate
// Method: __set_calibration
// Method: __get_calibration
// Method: getPoints
// Method: getStatus
// Method: getID
// Method: getX
// Method: getY
// Method: __get_raw_x
// Method: __get_raw_y
// Method: getWeight
// Method: getArea
 */

//
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