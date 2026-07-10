#include "../../slider.h"
#include "../core/internal.h"
#include "render_util.h"
#include <stdlib.h>
#include <stdio.h>

#define MSF_GIF_IMPL
#include "../msf_gif.h"

static void gif_frame_from_surface(cairo_surface_t *sfc, unsigned char *rgba_out) {
    int w = cairo_image_surface_get_width(sfc);
    int h = cairo_image_surface_get_height(sfc);
    unsigned char *data = cairo_image_surface_get_data(sfc);
    int stride = cairo_image_surface_get_stride(sfc);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t *pixel = (uint32_t *)(data + y * stride);
            uint32_t argb = pixel[x];
            int off = (y * w + x) * 4;
            rgba_out[off + 0] = (argb >> 16) & 0xFF;
            rgba_out[off + 1] = (argb >> 8) & 0xFF;
            rgba_out[off + 2] = argb & 0xFF;
            rgba_out[off + 3] = (argb >> 24) & 0xFF;
        }
    }
}

int slider_export_gif(Slider *s, const char *path, int w, int h) {
    if (!s || s->n_slides <= 0) return -1;

    int fps = 15;
    int hold_frames = fps;
    int trans_ms = TRANSITION_DEFAULT_MS;
    int trans_frames = (int)(trans_ms / 1000.0 * fps + 0.5);
    if (trans_frames < 1) trans_frames = 1;
    int delay_cs = 100 / fps;

    cairo_surface_t *sfc = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
    cairo_t *cr = cairo_create(sfc);

    unsigned char *frame_rgba = malloc(w * h * 4);
    if (!frame_rgba) { cairo_destroy(cr); cairo_surface_destroy(sfc); return -1; }

    MsfGifState gs = {};
    if (!msf_gif_begin(&gs, w, h)) {
        free(frame_rgba);
        cairo_destroy(cr);
        cairo_surface_destroy(sfc);
        return -1;
    }

    for (int i = 0; i < s->n_slides; i++) {
        if (i == 0) {
            for (int f = 0; f < hold_frames; f++) {
                SET_COLOR(cr, s->theme->bg);
                cairo_paint(cr);
                TransitionType saved = s->slides[i].transition;
                s->slides[i].transition = TRANS_NONE;
                slider_render(s, i, cr, w, h, (double)trans_ms + 1.0);
                s->slides[i].transition = saved;
                cairo_surface_flush(sfc);
                gif_frame_from_surface(sfc, frame_rgba);
                msf_gif_frame(&gs, frame_rgba, delay_cs, 16, 0);
            }
        } else {
            TransitionType trans = s->slides[i].transition;
            if (trans == TRANS_NONE) {
                for (int f = 0; f < hold_frames; f++) {
                    SET_COLOR(cr, s->theme->bg);
                    cairo_paint(cr);
                    TransitionType saved = s->slides[i].transition;
                    s->slides[i].transition = TRANS_NONE;
                    slider_render(s, i, cr, w, h, (double)trans_ms + 1.0);
                    s->slides[i].transition = saved;
                    cairo_surface_flush(sfc);
                    gif_frame_from_surface(sfc, frame_rgba);
                    msf_gif_frame(&gs, frame_rgba, delay_cs, 16, 0);
                }
            } else {
                for (int f = 0; f < trans_frames; f++) {
                    double t = (double)f / trans_frames * trans_ms;
                    SET_COLOR(cr, s->theme->bg);
                    cairo_paint(cr);
                    s->transition_type = trans;
                    s->transition_from = i - 1;
                    slider_render(s, i, cr, w, h, t);
                    cairo_surface_flush(sfc);
                    gif_frame_from_surface(sfc, frame_rgba);
                    msf_gif_frame(&gs, frame_rgba, delay_cs, 16, 0);
                }
                for (int f = 0; f < hold_frames; f++) {
                    SET_COLOR(cr, s->theme->bg);
                    cairo_paint(cr);
                    TransitionType saved = s->slides[i].transition;
                    s->slides[i].transition = TRANS_NONE;
                    slider_render(s, i, cr, w, h, (double)trans_ms + 1.0);
                    s->slides[i].transition = saved;
                    cairo_surface_flush(sfc);
                    gif_frame_from_surface(sfc, frame_rgba);
                    msf_gif_frame(&gs, frame_rgba, delay_cs, 16, 0);
                }
            }
        }
    }

    MsfGifResult result = msf_gif_end(&gs);
    int ok = 0;
    if (result.data) {
        FILE *fp = fopen(path, "wb");
        if (fp) {
            ok = fwrite(result.data, result.dataSize, 1, fp) == 1;
            fclose(fp);
        }
    }
    msf_gif_free(result);

    free(frame_rgba);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return ok ? 0 : -1;
}
