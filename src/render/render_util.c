#include "render_util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR
#include "../stb_image.h"

static ImgCache img_cache[MAX_IMG_CACHE];
static int      img_cache_count = 0;
static unsigned long img_cache_tick = 0;

static void evict_lru(void) {
    int oldest = 0;
    for (int i = 1; i < img_cache_count; i++) {
        if (img_cache[i].last_access < img_cache[oldest].last_access)
            oldest = i;
    }
    ImgCache *c = &img_cache[oldest];
    if (c->surfaces) {
        for (int f = 0; f < c->n_frames; f++)
            if (c->surfaces[f]) cairo_surface_destroy(c->surfaces[f]);
        free(c->surfaces);
    }
    free(c->delays);
    /* compact: move last slot into evicted slot */
    img_cache[oldest] = img_cache[img_cache_count - 1];
    img_cache_count--;
}

static cairo_surface_t *create_cairo_surface_from_stbi(unsigned char *data, int w, int h, int channels) {
    if (!data) return NULL;
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w);
    unsigned char *surface_data = malloc(stride * h);
    if (!surface_data) return NULL;

    for (int y = 0; y < h; y++) {
        uint32_t *row = (uint32_t *)(surface_data + y * stride);
        for (int x = 0; x < w; x++) {
            unsigned char *src = data + (y * w + x) * channels;
            uint8_t r = 0, g = 0, b = 0, a = 255;

            if (channels == 1) {
                r = g = b = src[0];
            } else if (channels == 2) {
                r = g = b = src[0];
                a = src[1];
            } else if (channels == 3) {
                r = src[0]; g = src[1]; b = src[2];
            } else if (channels == 4) {
                r = src[0]; g = src[1]; b = src[2]; a = src[3];
            }

            if (a != 255) {
                r = (r * a) / 255;
                g = (g * a) / 255;
                b = (b * a) / 255;
            }

            row[x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    cairo_surface_t *s = cairo_image_surface_create_for_data(
        surface_data, CAIRO_FORMAT_ARGB32, w, h, stride
    );

    if (cairo_surface_status(s) != CAIRO_STATUS_SUCCESS) {
        free(surface_data);
        return NULL;
    }

    static cairo_user_data_key_t key;
    cairo_surface_set_user_data(s, &key, surface_data, free);

    return s;
}

ImgCache *get_image_cache(const char *path) {
    for (int i = 0; i < img_cache_count; i++) {
        if (strcmp(img_cache[i].path, path) == 0) {
            img_cache[i].last_access = ++img_cache_tick;
            return &img_cache[i];
        }
    }

    if (img_cache_count >= MAX_IMG_CACHE) evict_lru();

    ImgCache *cache = &img_cache[img_cache_count];
    strncpy(cache->path, path, 511);

    int len = strlen(path);
    int is_gif = (len > 4 && strcasecmp(path + len - 4, ".gif") == 0);

    if (is_gif) {
        FILE *f = fopen(path, "rb");
        if (!f) { fprintf(stderr, "No se pudo abrir GIF: %s\n", path); return NULL; }

        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        unsigned char *buffer = malloc(fsize);
        if (fread(buffer, 1, fsize, f) != (size_t)fsize) {
            fclose(f); free(buffer); return NULL;
        }
        fclose(f);

        int *delays = NULL;
        int x, y, z, comp;
        unsigned char *data = stbi_load_gif_from_memory(buffer, fsize, &delays, &x, &y, &z, &comp, 4);
        free(buffer);

        if (!data) {
            fprintf(stderr, "Error cargando GIF: %s (%s)\n", path, stbi_failure_reason());
            return NULL;
        }

        cache->n_frames = z;
        cache->delays = delays;
        cache->surfaces = malloc(sizeof(cairo_surface_t*) * z);
        cache->total_duration = 0;

        int frame_size = x * y * 4;
        for (int i = 0; i < z; i++) {
            cache->surfaces[i] = create_cairo_surface_from_stbi(data + i * frame_size, x, y, 4);
            cache->total_duration += delays[i];
        }

        stbi_image_free(data);

    } else {
        int w, h, c;
        unsigned char *data = stbi_load(path, &w, &h, &c, 4);
        if (!data) {
            fprintf(stderr, "No se pudo cargar imagen: %s (%s)\n", path, stbi_failure_reason());
            return NULL;
        }

        cache->n_frames = 1;
        cache->surfaces = malloc(sizeof(cairo_surface_t*));
        cache->surfaces[0] = create_cairo_surface_from_stbi(data, w, h, 4);
        cache->delays = NULL;
        cache->total_duration = 0;

        stbi_image_free(data);
    }

    cache->last_access = ++img_cache_tick;
    img_cache_count++;
    return cache;
}

void img_cache_free_all(void) {
    for (int i = 0; i < img_cache_count; i++) {
        ImgCache *c = &img_cache[i];
        if (c->surfaces) {
            for (int f = 0; f < c->n_frames; f++) {
                if (c->surfaces[f])
                    cairo_surface_destroy(c->surfaces[f]);
            }
            free(c->surfaces);
        }
        free(c->delays);
        c->path[0] = '\0';
        c->n_frames = 0;
        c->surfaces = NULL;
        c->delays = NULL;
        c->total_duration = 0;
    }
    img_cache_count = 0;
    img_cache_tick = 0;
}

PangoLayout *make_layout(cairo_t *cr, const char *font_desc_str, double max_width_px) {
    PangoLayout *l = pango_cairo_create_layout(cr);
    PangoFontDescription *fd = pango_font_description_from_string(font_desc_str);
    pango_layout_set_font_description(l, fd);
    pango_font_description_free(fd);
    if (max_width_px > 0)
        pango_layout_set_width(l, (int)(max_width_px * PANGO_SCALE));
    pango_layout_set_wrap(l, PANGO_WRAP_WORD_CHAR);
    return l;
}

void set_color(cairo_t *cr, double r, double g, double b) {
    cairo_set_source_rgb(cr, r, g, b);
}

void md_to_markup(const char *in, char *out, size_t out_size) {
    size_t wi = 0;
    const char *p = in;
    int bold = 0;
    int italic = 0;
    int code = 0;

#define WRITE(s) do { \
    size_t _l = strlen(s); \
    if (wi + _l < out_size - 1) { memcpy(out + wi, s, _l); wi += _l; } \
} while(0)

    while (*p) {
        if (*p == '&') { WRITE("&amp;"); p++; continue; }
        if (*p == '<') { WRITE("&lt;"); p++; continue; }
        if (*p == '>') { WRITE("&gt;"); p++; continue; }

        if (*p == '`') {
            if (!code) { WRITE("<tt>"); code = 1; }
            else { WRITE("</tt>"); code = 0; }
            p++; continue;
        }

        if (code) {
            if (wi < out_size - 1) out[wi++] = *p;
            p++; continue;
        }

        if (strncmp(p, "***", 3) == 0 || strncmp(p, "___", 3) == 0) {
            if (!bold && !italic) { WRITE("<b><i>"); bold = 1; italic = 1; }
            else if (bold && italic) { WRITE("</i></b>"); bold = 0; italic = 0; }
            p += 3; continue;
        }
        if (strncmp(p, "**", 2) == 0 || strncmp(p, "__", 2) == 0) {
            if (!bold) { WRITE("<b>"); bold = 1; }
            else { WRITE("</b>"); bold = 0; }
            p += 2; continue;
        }
        if (*p == '*' || *p == '_') {
            if (!italic) { WRITE("<i>"); italic = 1; }
            else { WRITE("</i>"); italic = 0; }
            p++; continue;
        }

        if (wi < out_size - 1) out[wi++] = *p;
        p++;
    }
    if (italic) WRITE("</i>");
    if (bold) WRITE("</b>");

    out[wi] = '\0';
#undef WRITE
}

double render_pango(cairo_t *cr, PangoLayout *lay, const char *text, double x, double y) {
    static char markup[MAX_LINE_LEN * 4];
    md_to_markup(text, markup, sizeof(markup));
    pango_layout_set_markup(lay, markup, -1);
    int tw, th;
    pango_layout_get_pixel_size(lay, &tw, &th);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, lay);
    return (double)th;
}
