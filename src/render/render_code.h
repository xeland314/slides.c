#ifndef RENDER_CODE_H
#define RENDER_CODE_H

#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include "../core/internal.h"
#include "../../slider.h"

double render_code_block(cairo_t *cr, PangoLayout *lay_code,
                         Slider *s, SlideLine *lines, int start, int count,
                         double x, double y, double max_w);

#endif
