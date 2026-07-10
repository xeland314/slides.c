#include "../../slider.h"
#include "../core/internal.h"
#include "render_util.h"

int slider_export_png(Slider *s, int index, const char *path, int w, int h) {
    if (!s || index < 0 || index >= s->n_slides) return -1;

    cairo_surface_t *sfc = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
    cairo_t *cr = cairo_create(sfc);

    SET_COLOR(cr, s->theme->bg);
    cairo_paint(cr);

    TransitionType saved = s->slides[index].transition;
    s->slides[index].transition = TRANS_NONE;
    slider_render(s, index, cr, w, h, 0.0);
    s->slides[index].transition = saved;

    cairo_status_t status = cairo_surface_write_to_png(sfc, path);

    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return (status == CAIRO_STATUS_SUCCESS) ? 0 : -1;
}
