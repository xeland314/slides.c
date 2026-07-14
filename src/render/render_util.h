#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include "../core/internal.h"

#define SET_COLOR(cr, c) set_color((cr), COLOR_R(c), COLOR_G(c), COLOR_B(c))

PangoLayout *make_layout(cairo_t *cr, const char *font_desc_str, double max_width_px);
void set_color(cairo_t *cr, double r, double g, double b);
void md_to_markup(const char *in, char *out, size_t out_size);
double render_pango(cairo_t *cr, PangoLayout *lay, const char *text, double x, double y);
ImgCache *get_image_cache(const char *path);
void img_cache_free_all(void);

#endif
