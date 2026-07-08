#ifndef RENDER_TRANSITION_H
#define RENDER_TRANSITION_H

#include <cairo/cairo.h>
#include "../core/internal.h"
#include "../../slider.h"

void do_transition(Slider *s, int from_idx, int to_idx, cairo_t *cr,
                   int win_w, int win_h, double progress);

#endif
