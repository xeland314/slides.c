#include "../../slider.h"
#include "../core/internal.h"
#include <pango/pangocairo.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

static ImgCache img_cache[MAX_IMG_CACHE];
static int      img_cache_count = 0;

// ── Cache de imágenes ─────────────────────────────────────────────────────────

static cairo_surface_t *get_image(const char *path) {
    for (int i = 0; i < img_cache_count; i++)
        if (strcmp(img_cache[i].path, path) == 0)
            return img_cache[i].surface;

    if (img_cache_count >= MAX_IMG_CACHE) return NULL;

    cairo_surface_t *s = cairo_image_surface_create_from_png(path);
    if (cairo_surface_status(s) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "No se pudo cargar imagen: %s\n", path);
        cairo_surface_destroy(s);
        return NULL;
    }
    strncpy(img_cache[img_cache_count].path, path, 511);
    img_cache[img_cache_count].surface = s;
    img_cache_count++;
    return s;
}

// ── Utilidades de render ──────────────────────────────────────────────────────

static PangoLayout *make_layout(cairo_t *cr, const char *font_desc_str,
                                 double max_width_px) {
    PangoLayout *l = pango_cairo_create_layout(cr);
    PangoFontDescription *fd = pango_font_description_from_string(font_desc_str);
    pango_layout_set_font_description(l, fd);
    pango_font_description_free(fd);
    if (max_width_px > 0)
        pango_layout_set_width(l, (int)(max_width_px * PANGO_SCALE));
    pango_layout_set_wrap(l, PANGO_WRAP_WORD_CHAR);
    return l;
}

static void set_color(cairo_t *cr, double r, double g, double b) {
    cairo_set_source_rgb(cr, r, g, b);
}

static void md_to_markup(const char *in, char *out, size_t out_size) {
    size_t wi = 0;
    const char *p = in;

#define WRITE(s) do { \
    size_t _l = strlen(s); \
    if (wi + _l < out_size - 1) { memcpy(out + wi, s, _l); wi += _l; } \
} while(0)

    while (*p) {
        if (*p == '&') { WRITE("&amp;");  p++; continue; }
        if (*p == '<') { WRITE("&lt;");   p++; continue; }
        if (*p == '>') { WRITE("&gt;");   p++; continue; }

        if ((p[0] == '*' && p[1] == '*') || (p[0] == '_' && p[1] == '_')) {
            char delim[3] = { p[0], p[1], '\0' };
            const char *end = strstr(p + 2, delim);
            if (end) {
                WRITE("<b>");
                for (const char *q = p + 2; q < end; q++) {
                    if (*q == '&') WRITE("&amp;");
                    else if (*q == '<') WRITE("&lt;");
                    else if (*q == '>') WRITE("&gt;");
                    else { if (wi < out_size - 1) out[wi++] = *q; }
                }
                WRITE("</b>");
                p = end + 2;
                continue;
            }
        }

        if (p[0] == '*' && p[1] != '*') {
            const char *end = strchr(p + 1, '*');
            if (end && end > p + 1) {
                WRITE("<i>");
                for (const char *q = p + 1; q < end; q++) {
                    if (*q == '&') WRITE("&amp;");
                    else if (*q == '<') WRITE("&lt;");
                    else if (*q == '>') WRITE("&gt;");
                    else { if (wi < out_size - 1) out[wi++] = *q; }
                }
                WRITE("</i>");
                p = end + 1;
                continue;
            }
        }

        if (p[0] == '_' && p[1] != '_') {
            const char *end = strchr(p + 1, '_');
            if (end && end > p + 1) {
                WRITE("<i>");
                for (const char *q = p + 1; q < end; q++) {
                    if (*q == '&') WRITE("&amp;");
                    else if (*q == '<') WRITE("&lt;");
                    else if (*q == '>') WRITE("&gt;");
                    else { if (wi < out_size - 1) out[wi++] = *q; }
                }
                WRITE("</i>");
                p = end + 1;
                continue;
            }
        }

        if (wi < out_size - 1) out[wi++] = *p;
        p++;
    }
    out[wi] = '\0';
#undef WRITE
}

static double render_pango(cairo_t *cr, PangoLayout *lay,
                            const char *text, double x, double y) {
    char markup[MAX_LINE_LEN * 4];
    md_to_markup(text, markup, sizeof(markup));
    pango_layout_set_markup(lay, markup, -1);
    int tw, th;
    pango_layout_get_pixel_size(lay, &tw, &th);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, lay);
    return (double)th;
}

static double render_table(cairo_t *cr, PangoLayout *lay_body,
                            SlideLine *lines, int start, int count,
                            double x, double y, double max_w) {
    if (count == 0) return 0.0;
    int max_cols = 0, header_row = -1;
    for (int i = 0; i < count; i++) {
        if (lines[start + i].type == LINE_TABLE_ROW && lines[start + i].ncols > max_cols)
            max_cols = lines[start + i].ncols;
        if (lines[start + i].type == LINE_TABLE_SEP) header_row = i;
    }
    if (max_cols == 0) return 0.0;

    double col_w = (max_w - 2.0) / max_cols;
    double row_h = 40.0;
    double cur_y = y;
    int data_row = 0;

    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_TABLE_SEP) continue;
        if (sl->type != LINE_TABLE_ROW) continue;
        int is_header = (header_row > 0 && i == 0);

        if (is_header) set_color(cr, COL_TABLE_HDR_R, COL_TABLE_HDR_G, COL_TABLE_HDR_B);
        else if (data_row % 2 == 0) set_color(cr, COL_TABLE_ROW_R, COL_TABLE_ROW_G, COL_TABLE_ROW_B);
        else set_color(cr, COL_TABLE_ALT_R, COL_TABLE_ALT_G, COL_TABLE_ALT_B);

        cairo_rectangle(cr, x, cur_y, max_w, row_h);
        cairo_fill(cr);
        set_color(cr, COL_TABLE_BDR_R, COL_TABLE_BDR_G, COL_TABLE_BDR_B);
        cairo_set_line_width(cr, 0.5);
        for (int c = 0; c <= max_cols; c++) {
            double cx = x + c * col_w;
            cairo_move_to(cr, cx, cur_y);
            cairo_line_to(cr, cx, cur_y + row_h);
            cairo_stroke(cr);
        }
        cairo_move_to(cr, x, cur_y + row_h);
        cairo_line_to(cr, x + max_w, cur_y + row_h);
        cairo_stroke(cr);

        if (is_header) set_color(cr, COL_BULLET_R, COL_BULLET_G, COL_BULLET_B);
        else set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);

        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            pango_layout_set_width(lay_body, (int)((col_w - 12.0) * PANGO_SCALE));
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            cairo_move_to(cr, x + c * col_w + 6.0, cur_y + (row_h - (double)th) / 2.0);
            pango_cairo_show_layout(cr, lay_body);
        }
        cur_y += row_h;
        if (!is_header) data_row++;
    }
    set_color(cr, COL_TABLE_BDR_R, COL_TABLE_BDR_G, COL_TABLE_BDR_B);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, x, y, max_w, cur_y - y);
    cairo_stroke(cr);
    pango_layout_set_width(lay_body, (int)((max_w) * PANGO_SCALE));
    return cur_y - y;
}

void slider_render(Slider *s, int index, cairo_t *cr, int win_w, int win_h) {
    if (!s || index < 0 || index >= s->n_slides) return;
    const Slide *slide = &s->slides[index];
    double content_w = win_w - MARGIN_X * 2.0;

    PangoLayout *lay_title    = make_layout(cr, "Inter Bold 44",    content_w);
    PangoLayout *lay_subtitle = make_layout(cr, "Inter 26",         content_w);
    PangoLayout *lay_body     = make_layout(cr, "Inter 20",         content_w);
    PangoLayout *lay_bullet   = make_layout(cr, "Inter 20",         content_w - 30);
    PangoLayout *lay_bullet2  = make_layout(cr, "Inter 18",         content_w - 60);
    PangoLayout *lay_num      = make_layout(cr, "Inter 13",         200);

    double y = MARGIN_Y;
    int i = 0;
    while (i < slide->nlines) {
        const SlideLine *sl = &slide->lines[i];
        switch (sl->type) {
        case LINE_EMPTY: y += 12.0; i++; break;
        case LINE_TITLE:
            set_color(cr, COL_TITLE_R, COL_TITLE_G, COL_TITLE_B);
            y += render_pango(cr, lay_title, sl->text, MARGIN_X, y);
            set_color(cr, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B);
            cairo_set_line_width(cr, 2.5);
            cairo_move_to(cr, MARGIN_X, y + 8);
            cairo_line_to(cr, MARGIN_X + content_w, y + 8);
            cairo_stroke(cr);
            y += 22.0; i++; break;
        case LINE_SUBTITLE:
            set_color(cr, COL_SUB_R, COL_SUB_G, COL_SUB_B);
            y += render_pango(cr, lay_subtitle, sl->text, MARGIN_X, y);
            y += 14.0; i++; break;
        case LINE_BODY:
            set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);
            y += render_pango(cr, lay_body, sl->text, MARGIN_X, y);
            y += 8.0; i++; break;
        case LINE_BLOCKQUOTE: {
            double b_x = MARGIN_X + 10.0;
            double t_x = b_x + 25.0;
            set_color(cr, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B);
            cairo_set_line_width(cr, 4.0);
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            cairo_move_to(cr, b_x, y);
            cairo_line_to(cr, b_x, y + (double)th);
            cairo_stroke(cr);
            set_color(cr, COL_SUB_R, COL_SUB_G, COL_SUB_B);
            cairo_move_to(cr, t_x, y);
            pango_cairo_show_layout(cr, lay_body);
            y += (double)th + 12.0;
            i++; break;
        }
        case LINE_BULLET1:
            set_color(cr, COL_BULLET_R, COL_BULLET_G, COL_BULLET_B);
            cairo_arc(cr, MARGIN_X + 8, y + 11, 4, 0, 2 * M_PI);
            cairo_fill(cr);
            set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);
            y += render_pango(cr, lay_bullet, sl->text, MARGIN_X + 22.0, y) + 6.0;
            i++; break;
        case LINE_BULLET2: {
            double bx = MARGIN_X + 38, by = y + 10;
            set_color(cr, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B);
            cairo_move_to(cr, bx, by - 4); cairo_line_to(cr, bx + 4, by);
            cairo_line_to(cr, bx, by + 4); cairo_line_to(cr, bx - 4, by);
            cairo_close_path(cr); cairo_fill(cr);
            set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);
            y += render_pango(cr, lay_bullet2, sl->text, MARGIN_X + 52.0, y) + 4.0;
            i++; break;
        }
        case LINE_IMAGE: {
            cairo_surface_t *img = get_image(sl->text);
            if (img) {
                int iw = cairo_image_surface_get_width(img), ih = cairo_image_surface_get_height(img);
                double avail_h = win_h - y - MARGIN_Y - 40.0, scale = 1.0;
                if (iw > content_w) scale = content_w / iw;
                if (ih * scale > avail_h) scale = avail_h / ih;
                double dw = iw * scale, dh = ih * scale;
                cairo_save(cr); cairo_translate(cr, MARGIN_X + (content_w - dw) / 2.0, y);
                cairo_scale(cr, scale, scale); cairo_set_source_surface(cr, img, 0, 0);
                cairo_paint(cr); cairo_restore(cr);
                y += dh + 14.0;
            } else {
                set_color(cr, 0.15, 0.18, 0.38); cairo_rectangle(cr, MARGIN_X, y, content_w, 80); cairo_fill(cr);
                set_color(cr, COL_LABEL_R, COL_LABEL_G, COL_LABEL_B);
                char msg[MAX_LINE_LEN + 32]; snprintf(msg, sizeof(msg), "⚠ No se encontró: %s", sl->text);
                render_pango(cr, lay_body, msg, MARGIN_X + 10, y + 28);
                y += 94.0;
            }
            i++; break;
        }
        case LINE_TABLE_ROW:
        case LINE_TABLE_SEP: {
            int j = i;
            while (j < slide->nlines && (slide->lines[j].type == LINE_TABLE_ROW || slide->lines[j].type == LINE_TABLE_SEP)) j++;
            y += render_table(cr, lay_body, (SlideLine *)slide->lines, i, j - i, MARGIN_X, y, content_w) + 14.0;
            i = j; break;
        }
        default: i++; break;
        }
    }
    char num_buf[32]; snprintf(num_buf, sizeof(num_buf), "%d / %d", index + 1, s->n_slides);
    set_color(cr, COL_NUM_R, COL_NUM_G, COL_NUM_B);
    pango_layout_set_width(lay_num, 200 * PANGO_SCALE); pango_layout_set_alignment(lay_num, PANGO_ALIGN_RIGHT);
    render_pango(cr, lay_num, num_buf, win_w - MARGIN_X - 200, win_h - 32.0);
    double prog = (s->n_slides > 1) ? (double)index / (s->n_slides - 1) : 1.0;
    set_color(cr, 0.10, 0.14, 0.35); cairo_rectangle(cr, 0, win_h - 4, win_w, 4); cairo_fill(cr);
    set_color(cr, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B); cairo_rectangle(cr, 0, win_h - 4, win_w * prog, 4); cairo_fill(cr);
    g_object_unref(lay_title); g_object_unref(lay_subtitle); g_object_unref(lay_body);
    g_object_unref(lay_bullet); g_object_unref(lay_bullet2); g_object_unref(lay_num);
}
