#ifndef RENDER_TABLE_H
#define RENDER_TABLE_H

#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include "../core/internal.h"
#include "../../slider.h"

double render_table(cairo_t *cr, PangoLayout *lay_body,
                    Slider *s, SlideLine *lines, int start, int count,
                    double x, double y, double max_w);

#endif
