#include "../../slider.h"
#include "../core/internal.h"
#include "render_util.h"
#include <cairo/cairo-svg.h>

int slider_export_svg(Slider *s, int index, const char *path, int w, int h) {
    if (!s || index < 0 || index >= s->n_slides) return -1;

    cairo_surface_t *sfc = cairo_svg_surface_create(path, (double)w, (double)h);
    if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS) {
        return -1;
    }

    cairo_t *cr = cairo_create(sfc);

    set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
    cairo_paint(cr);

    TransitionType saved = s->slides[index].transition;
    s->slides[index].transition = TRANS_NONE;
    slider_render(s, index, cr, w, h, 0.0);
    s->slides[index].transition = saved;

    cairo_status_t status = cairo_surface_status(sfc);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return (status == CAIRO_STATUS_SUCCESS) ? 0 : -1;
}
