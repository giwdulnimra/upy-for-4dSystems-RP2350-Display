extern "C" {
#include "py/obj.h"
#include "py/runtime.h"
//#include "py/builtin.h"
}
#include "Graphics4D.h"
extern const uint8_t Font1[];
extern const uint8_t Font2[];
extern const uint8_t Font3[];
extern const uint8_t Font4[];

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

// Method: setFont(font: int)
STATIC mp_obj_t mp_graphics4d_set_font(mp_obj_t font_id_obj) {
    GFX_CHECK(self);
    int font_id = mp_obj_get_int(font_id_obj);
    const uint8_t *font_ptr = NULL;
    switch (font_id) {
        case 1: font_ptr = Font1; break;
        case 2: font_ptr = Font2; break;
        case 3: font_ptr = Font3; break;
        case 4: font_ptr = Font4; break;
        default: break;
    }

    Graphics4D* gfx = get_graphics4d_instance();
    const uint8_t *last_font = gfx->SetFont(font_ptr);
    return mp_obj_new_int_from_uint((uintptr_t)last_font);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_graphics4d_set_font_obj, mp_graphics4d_set_font);

// Method: setFontForeground(color)
STATIC mp_obj_t mp_graphics4d_set_font_fg(mp_obj_t color_obj) {
    GFX_CHECK(self);
    uint16_t color = mp_obj_get_int(color_obj);
    uint16_t old_color = gfx->SetFontForeground(color);
    return mp_obj_new_int(old_color);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_graphics4d_set_font_fg_obj, mp_graphics4d_set_font_fg);

// Method: setFontBackground(color)
STATIC mp_obj_t mp_graphics4d_set_font_bg(mp_obj_t color_obj) {
    GFX_CHECK(self);
    uint16_t color = mp_obj_get_int(color_obj);
    uint16_t old_color = gfx->SetFontBackground(color);
    return mp_obj_new_int(old_color);
}

// Method: getStringWidth(string: str)
STATIC mp_obj_t mp_graphics4d_get_string_width(mp_obj_t text_obj) {
    GFX_CHECK(self);
    const char *ts = mp_obj_str_get_str(text_obj);
    uint width = gfx->GetStringWidth(ts);
    return mp_obj_new_int(width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_graphics4d_get_string_width_obj, mp_graphics4d_get_string_width);

// Method: getFontHeight
STATIC mp_obj_t mp_graphics4d_get_font_height(void) {
    Graphics4D* gfx = get_graphics4d_instance();
    uint height = gfx->GetFontHeight();
    return mp_obj_new_int(height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_graphics4d_get_font_height_obj, mp_graphics4d_get_font_height);

// ~ __putch


// Method: print(text, draw_fb=True) || print(textarea, text, draw_fb_True)
STATIC mp_obj_t graphics4d_print(size_t n_args, const mp_obj_t *args) {
    //mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    // Case: print(str, draw_fb)
    if (n_args == 1 || (n_args == 2 && mp_obj_is_str(args[0]))) {
        const char *str = mp_obj_str_get_str(args[0]);
        bool draw_fb = (n_args == 2) ? mp_obj_is_true(args[1]) : true;
        size_t printed = gfx->print(str, draw_fb);
        return mp_obj_new_int(printed);
    }
    // Case: print(text_area, str, draw_fb)
    if (n_args >= 2 && mp_obj_is_type(args[0], &text_area_type)) {
        mp_obj_textarea4d_t *ta = MP_OBJ_TO_PTR(args[0]);
        const char *str = mp_obj_str_get_str(args[1]);
        bool draw_fb = (n_args == 3) ? mp_obj_is_true(args[2]) : true;
        size_t printed = gfx->print(ta->area, str, draw_fb);
        return mp_obj_new_int(printed);
    }
    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_print_obj, 1, 3, graphics4d_print);

// Method: printf(text, *args, draw_fb=True) || printf(textarea, text, *args, draw_fb_True)
STATIC mp_obj_t graphics4d_printf(size_t n_args, const mp_obj_t *args) {
    GFX_CHECK(self);
    // Case: printf(str, *args)
    if (n_args >= 2 && mp_obj_is_str(args[0])) {
        mp_obj_t formatted = mp_obj_str_format(n_args, args);
        const char *str = mp_obj_str_get_str(formatted);
        size_t printed = gfx->print(str);
        return mp_obj_new_int(printed);
    }
    // Case: printf(text_area, str, *args)
    if (n_args >= 3 && mp_obj_is_type(args[0], &text_area_type)) {
        mp_obj_textarea4d_t *ta = MP_OBJ_TO_PTR(args[0]);
        mp_obj_t formatted = mp_obj_str_format(n_args - 1, &args[1]);
        const char *str = mp_obj_str_get_str(formatted);
        size_t printed = gfx->print(ta->area, str);
        return mp_obj_new_int(printed);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_printf_obj, 2, 4, graphics4d_printf);

// Method: CreateTextArea(x1, y1, x2, y2, fg_color, bg_color)
STATIC mp_obj_t graphics4d_create_text_area(mp_obj_t x1_obj, mp_obj_t y1_obj,
                                            mp_obj_t x2_obj, mp_obj_t y2_obj,
                                            mp_obj_t fg_obj, mp_obj_t bg_obj) {
    GFX_CHECK(self);
    int x1 = mp_obj_get_int(x1_obj);
    int y1 = mp_obj_get_int(y1_obj);
    int x2 = mp_obj_get_int(x2_obj);
    int y2 = mp_obj_get_int(y2_obj);
    uint16_t fg = mp_obj_get_int(fg_obj);
    uint16_t bg = mp_obj_get_int(bg_obj);
    TextArea4D area = gfx->CreateTextArea(x1, y1, x2, y2, fg, bg);
    mp_obj_textarea_t *obj = m_new_obj(mp_obj_textarea_t);
    obj->base.type = &text_area_type;
    obj->ta = ta;
    return MP_OBJ_FROM_PTR(obj);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_6(graphics4d_create_text_area_obj, graphics4d_create_text_area);


/* Class: ImageControl - 2358 (?)

/*
GRAPHICSMEDIA4D
 (zweimal überladen)    einen Wrapper schreiben, der zur Laufzeit entscheidet, welche C++-Überladung aufzurufen ist
*/

// Method: LoadImageControl(filename, x,y) || LoadImageControl(gci_array)
STATIC const mp_obj_t graphics4d_load_image_control(size_t n_args, const mp_obj_t *args) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(args[0]);
    GFX_CHECK(self);
    if (n_args == 4) {
        // Overload 1: LoadImageControl(filename, x, y)
        const char *filename = mp_obj_str_get_str(args[1]);
        int x = mp_obj_get_int(args[2]);
        int y = mp_obj_get_int(args[3]);
        int index = self->gfx->LoadImageControl(filename, x, y);
        return mp_obj_new_int(index);
    } else if (n_args == 2) {
        // Overload 2: LoadImageControl(gci_array)
        mp_buffer_info bufinfo;
        mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);
        int index = self->gfx->LoadImageControl((const uint8_t *)bufinfo.buf);
        return mp_obj_new_int(index);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("load_image_control(filename, x, y) or load_image_control(gci_array)"));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(graphics4d_load_image_control_obj, 2, 4, graphics4d_load_image_control);

// Method: getCount()
STATIC mp_obj_t graphics4d_get_count(mp_obj_t self_in) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int count = self->gfx->GetCount();
    return mp_obj_new_int(count);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(graphics4d_get_count_obj, graphics4d_get_count);

// Method: getInfo(index)
STATIC mp_obj_t graphics4d_get_info(mp_obj_t self_in, mp_obj_t index_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    int info = self->gfx->GetInfo(index);
    return mp_obj_new_int(info);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_get_info_obj, graphics4d_get_info);

// Method: setProperties(index, properties_buf)
STATIC mp_obj_t graphics4d_set_properties(mp_obj_t self_in, mp_obj_t index_obj, mp_obj_t properties_buf_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    mp_buffer_info bufinfo;
    mp_get_buffer_raise(properties_buf_obj, &bufinfo, MP_BUFFER_READ);
    self->gfx->SetProperties(index, (const uint8_t *)bufinfo.buf);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(graphics4d_set_properties_obj, graphics4d_set_properties);

// Method: setValue(index, value)
STATIC mp_obj_t graphics4d_set_value(mp_obj_t self_in, mp_obj_t index_obj, mp_obj_t value_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    int value = mp_obj_get_int(value_obj);
    self->gfx->SetValue(index, value);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(graphics4d_set_value_obj, graphics4d_set_value);

// Method: getValue(index)
STATIC mp_obj_t graphics4d_get_value(mp_obj_t self_in, mp_obj_t index_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    int value = self->gfx->GetValue(index);
    return mp_obj_new_int(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_get_value_obj, graphics4d_get_value);

// Method: getFrames(index)
STATIC mp_obj_t graphics4d_get_frames(mp_obj_t self_in, mp_obj_t index_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    int frames = self->gfx->GetFrames(index);
    return mp_obj_new_int(frames);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_get_frames_obj, graphics4d_get_frames);

// Method: setPosition(index, x, y)
STATIC mp_obj_t graphics4d_set_position(mp_obj_t self_in, mp_obj_t index_obj, mp_obj_t x_obj, mp_obj_t y_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    int x = mp_obj_get_int(x_obj);
    int y = mp_obj_get_int(y_obj);
    self->gfx->SetPosition(index, x, y);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_4(graphics4d_set_position_obj, graphics4d_set_position);

// Method: Clear(index)
STATIC mp_obj_t graphics4d_clear_control(mp_obj_t self_in, mp_obj_t index_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    self->gfx->Clear(index);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_clear_control_obj, graphics4d_clear_control);

// Method: Show(index)
STATIC mp_obj_t graphics4d_show_control(mp_obj_t self_in, mp_obj_t index_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int index = mp_obj_get_int(index_obj);
    self->gfx->Show(index);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_show_control_obj, graphics4d_show_control);

// Method: ShowForm(form_id)
STATIC mp_obj_t graphics4d_show_form(mp_obj_t self_in, mp_obj_t form_id_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int form_id = mp_obj_get_int(form_id_obj);
    self->gfx->ShowForm(form_id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(graphics4d_show_form_obj, graphics4d_show_form);

// Method: Touched(x, y) -> bool
STATIC mp_obj_t graphics4d_touched(mp_obj_t self_in, mp_obj_t x_obj, mp_obj_t y_obj) {
    mp_obj_graphics4d_t *self = (mp_obj_graphics4d_t *)MP_OBJ_TO_PTR(self_in);
    GFX_CHECK(self);
    int x = mp_obj_get_int(x_obj);
    int y = mp_obj_get_int(y_obj);
    bool touched = self->gfx->Touched(x, y);
    return mp_const_bool(touched);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(graphics4d_touched_obj, graphics4d_touched);

// ~ __draw_to_buffer
// ~ __show_digits
// ~ __show_2_frame_gauge
// ~ __show_linear_gauge
// ~ __show_knob
// ~ __redraw_form_region


/*
GRAPHICSTOUCH4D
*/
// Method: Initialize
// Method: Calibrate
// ~ __set_calibration
// ~ __get_calibration
// Method: getPoints
// Method: getStatus
// Method: getID
// Method: getX
// Method: getY
// ~ __get_raw_x
// ~ __get_raw_y
// Method: getWeight
// Method: getArea

/*
GRAPHICSTOUCH4D
*/

typedef struct _mp_obj_touch4d_t {
    mp_obj_base_t base;
    GraphicsTouch4D *touch; // Pointer to the C++ GraphicsTouch4D singleton instance
} mp_obj_touch4d_t;

// Constructor for the Python Touch class: touch = graphics4d.Touch4D()
STATIC mp_obj_t touch4d_make_new(const mp_obj_type_t *type,
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

// Destructor / deinit method
STATIC mp_obj_t touch4d_deinit(mp_obj_t self_in) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(self_in);
    if (self->touch) {
        // We don't delete the singleton instance, just nullify the pointer
        self->touch = NULL; 
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(touch4d_deinit_obj, touch4d_deinit);
// close()-alias for deinit
STATIC mp_obj_t touch4d_close(mp_obj_t self_in) {
    return touch4d_deinit(self_in);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(touch4d_close_obj, touch4d_close);

// Convenience macro to check if the touch object is initialized
#define TOUCH_CHECK(self)     if (!self || !(self)->touch) {mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("GraphicsTouch4D not initialized"));}

// Method: Calibrate() -> bool
STATIC mp_obj_t touch4d_calibrate(mp_obj_t self_in) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(self_in);
    TOUCH_CHECK(self);
    bool result = self->touch->Calibrate();
    return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(touch4d_calibrate_obj, touch4d_calibrate);

// Method: getPoints() -> int
STATIC mp_obj_t touch4d_get_points(mp_obj_t self_in) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(self_in);
    TOUCH_CHECK(self);
    uint8_t points = self->touch->GetPoints();
    return mp_obj_new_int(points);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(touch4d_get_points_obj, touch4d_get_points);

// Method: getStatus(point=0) -> int
STATIC mp_obj_t touch4d_get_status(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int8_t status = self->touch->GetStatus(point);
    return mp_obj_new_int(status);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(touch4d_get_status_obj, 1, 2, touch4d_get_status);

// Method: getID(point=0) -> int
STATIC mp_obj_t touch4d_get_id(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t id = self->touch->GetID(point);
    return mp_obj_new_int(id);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(touch4d_get_id_obj, 1, 2, touch4d_get_id);

// Method: getX(point=0) -> int
STATIC mp_obj_t touch4d_get_x(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t x = self->touch->GetX(point);
    return mp_obj_new_int(x);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(touch4d_get_x_obj, 1, 2, touch4d_get_x);

// Method: getY(point=0) -> int
STATIC mp_obj_t touch4d_get_y(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t y = self->touch->GetY(point);
    return mp_obj_new_int(y);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(touch4d_get_y_obj, 1, 2, touch4d_get_y);

// Method: getWeight(point=0) -> int
STATIC mp_obj_t touch4d_get_weight(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t weight = self->touch->GetWeight(point);
    return mp_obj_new_int(weight);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(touch4d_get_weight_obj, 1, 2, touch4d_get_weight);

// Method: getArea(point=0) -> int
STATIC mp_obj_t touch4d_get_area(size_t n_args, const mp_obj_t *args) {
    mp_obj_touch4d_t *self = (mp_obj_touch4d_t *)MP_OBJ_TO_PTR(args[0]);
    TOUCH_CHECK(self);
    uint8_t point = (n_args > 1) ? mp_obj_get_int(args[1]) : 0;
    int16_t area = self->touch->GetArea(point);
    return mp_obj_new_int(area);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(touch4d_get_area_obj, 1, 2, touch4d_get_area);



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