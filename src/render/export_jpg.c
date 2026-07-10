#include "../../slider.h"
#include "../core/internal.h"
#include "render_util.h"
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"

int slider_export_jpg(Slider *s, int index, const char *path, int w, int h, int quality) {
    if (!s || index < 0 || index >= s->n_slides) return -1;

    cairo_surface_t *sfc = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
    cairo_t *cr = cairo_create(sfc);

    SET_COLOR(cr, s->theme->bg);
    cairo_paint(cr);

    TransitionType saved = s->slides[index].transition;
    s->slides[index].transition = TRANS_NONE;
    slider_render(s, index, cr, w, h, 0.0);
    s->slides[index].transition = saved;

    cairo_surface_flush(sfc);
    unsigned char *data = cairo_image_surface_get_data(sfc);
    int stride = cairo_image_surface_get_stride(sfc);

    unsigned char *rgb = malloc(w * h * 3);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int si = y * stride + x * 4;
            int di = (y * w + x) * 3;
            rgb[di + 0] = data[si + 2];
            rgb[di + 1] = data[si + 1];
            rgb[di + 2] = data[si + 0];
        }
    }

    int ok = stbi_write_jpg(path, w, h, 3, rgb, quality);

    free(rgb);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return ok ? 0 : -1;
}
