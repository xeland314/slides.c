#include "thememenu.h"
#include "../core/internal.h"
#include <pango/pangocairo.h>
#include <stdio.h>
#include <string.h>

#define HEADER_H 34.0
#define LABEL_H  24.0
#define PAD      8.0

int thememenu_count(const Tab *t) {
    return theme_count() + (t->base_custom ? 1 : 0);
}

const Theme *thememenu_theme(const Tab *t, int i) {
    if (t->base_custom) {
        if (i == 0) return &t->base_theme;
        i--;
    }
    return theme_at(i);
}

/* Entrada que corresponde al tema aplicado ahora mismo (o -1) */
static int current_entry(const Tab *t) {
    int n = thememenu_count(t);
    for (int i = 0; i < n; i++)
        if (theme_same_colors(thememenu_theme(t, i), t->s->theme))
            return i;
    return -1;
}

void thememenu_open(Tab *t) {
    int cur = current_entry(t);
    t->theme_sel = cur >= 0 ? cur : 0;
    t->theme_menu = 1;
    t->s->transition_type = TRANS_NONE;
}

void thememenu_apply(Tab *t, int i) {
    const Theme *th = thememenu_theme(t, i);
    if (th) {
        t->s->theme_storage = *th;
        t->s->theme = &t->s->theme_storage;
        fprintf(stderr, "[slides] Tema: %s%s\n",
                (t->base_custom && i == 0) ? "personalizado, base " : "",
                th->name ? th->name : "?");
    }
    t->theme_menu = 0;
    t->s->transition_type = TRANS_NONE;
}

int thememenu_move(Tab *t, int dx, int dy) {
    int n = thememenu_count(t);
    int sel = t->theme_sel + dx + dy * THEMEMENU_COLS;
    if (dy != 0 && (sel < 0 || sel >= n)) return 0;   /* sin envolver en vertical */
    if (sel < 0) sel = 0;
    if (sel >= n) sel = n - 1;
    if (sel == t->theme_sel) return 0;
    t->theme_sel = sel;
    return 1;
}

static void geometry(const Tab *t, int w, int h, int *rows, double *cw, double *ch) {
    int n = thememenu_count(t);
    *rows = (n + THEMEMENU_COLS - 1) / THEMEMENU_COLS;
    *cw = (double)w / THEMEMENU_COLS;
    *ch = (h - HEADER_H) / *rows;
}

static void label(cairo_t *cr, const char *font, const char *txt, double x,
                  double y, double max_w, double box_h, double r, double g,
                  double b) {
    PangoLayout *l = pango_cairo_create_layout(cr);
    PangoFontDescription *fd = pango_font_description_from_string(font);
    pango_layout_set_font_description(l, fd);
    pango_layout_set_text(l, txt, -1);
    pango_layout_set_width(l, (int)(max_w * PANGO_SCALE));
    pango_layout_set_ellipsize(l, PANGO_ELLIPSIZE_END);
    pango_layout_set_alignment(l, PANGO_ALIGN_CENTER);
    int tw, th;
    pango_layout_get_pixel_size(l, &tw, &th);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_move_to(cr, x, y + (box_h - th) / 2.0);
    pango_cairo_show_layout(cr, l);
    pango_font_description_free(fd);
    g_object_unref(l);
}

void thememenu_render(Tab *t, cairo_t *cr, int w, int h) {
    Slider *s = t->s;
    int n = thememenu_count(t);
    int rows;
    double cw, ch;
    geometry(t, w, h, &rows, &cw, &ch);
    int cur = current_entry(t);

    cairo_set_source_rgb(cr, 0.08, 0.08, 0.1);
    cairo_paint(cr);

    PangoLayout *hl = pango_cairo_create_layout(cr);
    PangoFontDescription *hfd = pango_font_description_from_string("Sans Bold 12");
    pango_layout_set_font_description(hl, hfd);
    pango_layout_set_text(hl, "TEMAS   [T/Esc cerrar · flechas mover · Enter/clic aplicar]", -1);
    cairo_set_source_rgb(cr, 0.6, 0.6, 0.65);
    cairo_move_to(cr, 12, 8);
    pango_cairo_show_layout(cr, hl);
    pango_font_description_free(hfd);
    g_object_unref(hl);

    /* Renderizar sin transición: slide "viene de sí misma" */
    const Theme *saved_theme = s->theme;
    int saved_from = s->transition_from;
    TransitionType saved_type = s->transition_type;
    s->transition_from = t->current;
    s->transition_type = TRANS_NONE;

    for (int i = 0; i < n; i++) {
        const Theme *th = thememenu_theme(t, i);
        int col = i % THEMEMENU_COLS, row = i / THEMEMENU_COLS;
        double cx = col * cw, cy = HEADER_H + row * ch;
        double thumb_h = ch - LABEL_H - 2 * PAD;
        if (thumb_h < 10) thumb_h = 10;
        double sx = (cw - 2 * PAD) / w, sy = thumb_h / h;
        double scl = sx < sy ? sx : sy;
        double tw = w * scl, tH = h * scl;
        double ox = cx + (cw - tw) / 2.0, oy = cy + PAD;

        s->theme = th;
        cairo_save(cr);
        cairo_translate(cr, ox, oy);
        cairo_scale(cr, scl, scl);
        cairo_rectangle(cr, 0, 0, w, h);
        cairo_clip(cr);
        cairo_set_source_rgb(cr, COLOR_R(th->bg), COLOR_G(th->bg), COLOR_B(th->bg));
        cairo_paint(cr);
        slider_render(s, t->current, cr, w, h, 0.0);
        cairo_restore(cr);

        cairo_rectangle(cr, ox - 2, oy - 2, tw + 4, tH + 4);
        if (i == t->theme_sel) {
            cairo_set_source_rgb(cr, 1, 0.84, 0);
            cairo_set_line_width(cr, 3);
        } else {
            cairo_set_source_rgb(cr, 0.3, 0.3, 0.35);
            cairo_set_line_width(cr, 1);
        }
        cairo_stroke(cr);

        char name[96];
        const char *tn = th->name ? th->name : "?";
        if (t->base_custom && i == 0)
            snprintf(name, sizeof(name), "personalizado (%s)", tn);
        else
            snprintf(name, sizeof(name), "%s", tn);
        if (i == cur) {
            size_t len = strlen(name);
            snprintf(name + len, sizeof(name) - len, "  \xE2\x97\x8F"); /* ● */
        }
        double g = (i == t->theme_sel) ? 0.95 : 0.62;
        label(cr, i == t->theme_sel ? "Sans Bold 10" : "Sans 10", name, cx + PAD,
              oy + tH + 2, cw - 2 * PAD, LABEL_H, g, g, g + 0.03);
    }

    s->theme = saved_theme;
    s->transition_from = saved_from;
    s->transition_type = saved_type;
}

int thememenu_hit(const Tab *t, int w, int h, int x, int y) {
    int rows;
    double cw, ch;
    geometry(t, w, h, &rows, &cw, &ch);
    if (y < HEADER_H || x < 0 || x >= w) return -1;
    int col = (int)(x / cw), row = (int)((y - HEADER_H) / ch);
    int i = row * THEMEMENU_COLS + col;
    return (col >= 0 && col < THEMEMENU_COLS && i >= 0 && i < thememenu_count(t)) ? i : -1;
}
