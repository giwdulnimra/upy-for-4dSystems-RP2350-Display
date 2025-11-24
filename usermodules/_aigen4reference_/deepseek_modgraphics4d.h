#ifndef MOD_GRAPHICS4D_H
#define MOD_GRAPHICS4D_H

#include "py/obj.h"
#include "py/runtime.h"

// MicroPython module definition
extern const mp_obj_module_t graphics4d_module;

// Graphics4D class
extern const mp_obj_type_t graphics4d_type;

// Function declarations
mp_obj_t graphics4d_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
mp_obj_t graphics4d_initialize(mp_obj_t self_in);
mp_obj_t graphics4d_cls(mp_obj_t self_in);
mp_obj_t graphics4d_rectangle(size_t n_args, const mp_obj_t *args);
mp_obj_t graphics4d_circle(size_t n_args, const mp_obj_t *args);
mp_obj_t graphics4d_line(size_t n_args, const mp_obj_t *args);
mp_obj_t graphics4d_put_pixel(size_t n_args, const mp_obj_t *args);
mp_obj_t graphics4d_set_backlight(mp_obj_t self_in, mp_obj_t level);
mp_obj_t graphics4d_get_width(mp_obj_t self_in);
mp_obj_t graphics4d_get_height(mp_obj_t self_in);

// Media functions
mp_obj_t graphics4d_load_image_control(size_t n_args, const mp_obj_t *args);
mp_obj_t graphics4d_show_image(mp_obj_t self_in, mp_obj_t handle, mp_obj_t index);

#endif