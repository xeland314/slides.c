#include "render_transition.h"
#include "render_util.h"
#include "../../slider.h"

void do_transition(Slider *s, int from_idx, int to_idx, cairo_t *cr,
                   int win_w, int win_h, double progress) {
    if (from_idx == to_idx || from_idx < 0 || from_idx >= s->n_slides) {
        slider_render(s, to_idx, cr, win_w, win_h, 0.0);
        return;
    }

    cairo_surface_t *target = cairo_get_target(cr);
    cairo_surface_t *sfc_from = cairo_surface_create_similar(target,
        CAIRO_CONTENT_COLOR, win_w, win_h);
    cairo_surface_t *sfc_to = cairo_surface_create_similar(target,
        CAIRO_CONTENT_COLOR, win_w, win_h);

    if (!sfc_from || !sfc_to) {
        if (sfc_from) cairo_surface_destroy(sfc_from);
        if (sfc_to) cairo_surface_destroy(sfc_to);
        slider_render(s, to_idx, cr, win_w, win_h, 0.0);
        return;
    }

    cairo_t *cr_from = cairo_create(sfc_from);
    cairo_t *cr_to = cairo_create(sfc_to);

    SET_COLOR(cr_from, s->theme->bg);
    cairo_paint(cr_from);
    SET_COLOR(cr_to, s->theme->bg);
    cairo_paint(cr_to);

    TransitionType saved_tt = s->transition_type;
    int saved_from = s->transition_from;
    TransitionType saved_from_trans = s->slides[from_idx].transition;
    TransitionType saved_to_trans = s->slides[to_idx].transition;
    s->transition_type = TRANS_NONE;
    s->slides[from_idx].transition = TRANS_NONE;
    s->slides[to_idx].transition = TRANS_NONE;

    slider_render(s, from_idx, cr_from, win_w, win_h, 0.0);
    slider_render(s, to_idx, cr_to, win_w, win_h, 0.0);

    s->transition_type = saved_tt;
    s->transition_from = saved_from;
    s->slides[from_idx].transition = saved_from_trans;
    s->slides[to_idx].transition = saved_to_trans;

    TransitionType tt = saved_tt;
    if (tt == TRANS_NONE) tt = s->slides[to_idx].transition;

    double p = progress;
    if (p < 0.0) p = 0.0;
    if (p > 1.0) p = 1.0;

    switch (tt) {
    case TRANS_FADE: {
        cairo_set_source_surface(cr, sfc_from, 0, 0);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, 0, 0);
        cairo_paint_with_alpha(cr, p);
        break;
    }
    case TRANS_SLIDE_LEFT: {
        double ox = (1.0 - p) * win_w;
        cairo_set_source_surface(cr, sfc_from, -win_w + ox, 0);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, ox, 0);
        cairo_paint(cr);
        break;
    }
    case TRANS_SLIDE_RIGHT: {
        double ox = -(1.0 - p) * win_w;
        cairo_set_source_surface(cr, sfc_from, win_w + ox, 0);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, ox, 0);
        cairo_paint(cr);
        break;
    }
    case TRANS_SLIDE_UP: {
        double oy = (1.0 - p) * win_h;
        cairo_set_source_surface(cr, sfc_from, 0, -win_h + oy);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, 0, oy);
        cairo_paint(cr);
        break;
    }
    case TRANS_SLIDE_DOWN: {
        double oy = -(1.0 - p) * win_h;
        cairo_set_source_surface(cr, sfc_from, 0, win_h + oy);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, 0, oy);
        cairo_paint(cr);
        break;
    }
    default:
        cairo_set_source_surface(cr, sfc_to, 0, 0);
        cairo_paint(cr);
        break;
    }

    cairo_destroy(cr_to);
    cairo_destroy(cr_from);
    cairo_surface_destroy(sfc_to);
    cairo_surface_destroy(sfc_from);
}
