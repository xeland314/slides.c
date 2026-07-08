#include "../../slider.h"
#include "../core/internal.h"
#include "render_util.h"
#include <cairo/cairo-pdf.h>

int slider_export_pdf(Slider *s, const char *path, int w, int h) {
    if (!s || s->n_slides <= 0) return -1;

    cairo_surface_t *sfc = cairo_pdf_surface_create(path, (double)w, (double)h);
    if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS) {
        return -1;
    }

    cairo_t *cr = cairo_create(sfc);

    for (int i = 0; i < s->n_slides; i++) {
        set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
        cairo_paint(cr);

        TransitionType saved = s->slides[i].transition;
        s->slides[i].transition = TRANS_NONE;
        slider_render(s, i, cr, w, h, 0.0);
        s->slides[i].transition = saved;

        cairo_show_page(cr);
    }

    cairo_status_t status = cairo_surface_status(sfc);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return (status == CAIRO_STATUS_SUCCESS) ? 0 : -1;
}
