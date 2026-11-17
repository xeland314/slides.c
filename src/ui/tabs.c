#include "tabs.h"
#include "../core/internal.h"
#include <limits.h>
#include <pango/pangocairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAB_MAX_W 220
#define TAB_MIN_W 70
#define PLUS_W    34
#define CLOSE_W   22

void tabs_init(TabSet *ts) { memset(ts, 0, sizeof(*ts)); }

void tabs_free(TabSet *ts) {
    for (int i = 0; i < ts->n; i++) slider_free(ts->tabs[i].s);
    ts->n = 0;
    ts->active = 0;
}

static void canon(const char *path, char *out, size_t n) {
    char r[PATH_MAX];
    if (realpath(path, r)) snprintf(out, n, "%s", r);
    else snprintf(out, n, "%s", path);
}

int tabs_find(const TabSet *ts, const char *path) {
    char a[PATH_MAX], b[PATH_MAX];
    canon(path, a, sizeof(a));
    for (int i = 0; i < ts->n; i++) {
        canon(ts->tabs[i].s->filepath, b, sizeof(b));
        if (strcmp(a, b) == 0) return i;
    }
    return -1;
}

int tabs_add(TabSet *ts, Slider *s, double now_ms) {
    int dup = tabs_find(ts, s->filepath);
    if (dup >= 0) {
        slider_free(s);
        ts->active = dup;
        return dup;
    }
    if (ts->n >= MAX_TABS) {
        slider_free(s);
        return -1;
    }
    Tab *t = &ts->tabs[ts->n];
    t->s = s;
    t->current = 0;
    t->slide_start_time = now_ms;
    t->zoom = 1.0;
    t->overview = 0;
    t->last_printed_slide = -1;
    thememenu_init(&t->tm, s);
    ts->active = ts->n++;
    return ts->active;
}

void tabs_close(TabSet *ts, int i) {
    if (i < 0 || i >= ts->n) return;
    slider_free(ts->tabs[i].s);
    for (int k = i; k < ts->n - 1; k++) ts->tabs[k] = ts->tabs[k + 1];
    ts->n--;
    if (ts->active > i || ts->active >= ts->n) ts->active--;
    if (ts->active < 0) ts->active = 0;
}

void tabs_next(TabSet *ts) { if (ts->n > 0) ts->active = (ts->active + 1) % ts->n; }
void tabs_prev(TabSet *ts) { if (ts->n > 0) ts->active = (ts->active + ts->n - 1) % ts->n; }
Tab *tabs_active(TabSet *ts) { return ts->n > 0 ? &ts->tabs[ts->active] : NULL; }

static double tab_width(const TabSet *ts, int w) {
    if (ts->n == 0) return TAB_MAX_W;
    double tw = (double)(w - PLUS_W) / ts->n;
    if (tw > TAB_MAX_W) tw = TAB_MAX_W;
    if (tw < TAB_MIN_W) tw = TAB_MIN_W;
    return tw;
}

static const char *base_name(const char *p) {
    const char *s = strrchr(p, '/');
    return s ? s + 1 : p;
}

static void text(cairo_t *cr, const char *font, const char *t, double x,
                 double y, double max_w, double r, double g, double b,
                 int center, double box_h) {
    PangoLayout *l = pango_cairo_create_layout(cr);
    PangoFontDescription *fd = pango_font_description_from_string(font);
    pango_layout_set_font_description(l, fd);
    pango_layout_set_text(l, t, -1);
    if (max_w > 0) {
        pango_layout_set_width(l, (int)(max_w * PANGO_SCALE));
        pango_layout_set_ellipsize(l, PANGO_ELLIPSIZE_END);
    }
    int tw, th;
    pango_layout_get_pixel_size(l, &tw, &th);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_move_to(cr, center ? x + (max_w - tw) / 2.0 : x, y + (box_h - th) / 2.0);
    pango_cairo_show_layout(cr, l);
    pango_font_description_free(fd);
    g_object_unref(l);
}

void tabs_render_bar(const TabSet *ts, cairo_t *cr, int w, int bar_h) {
    cairo_set_source_rgb(cr, 0.05, 0.05, 0.07);
    cairo_rectangle(cr, 0, 0, w, bar_h);
    cairo_fill(cr);
    double tw = tab_width(ts, w);
    for (int i = 0; i < ts->n; i++) {
        double x = i * tw;
        int act = (i == ts->active);
        cairo_set_source_rgb(cr, act ? 0.20 : 0.11, act ? 0.25 : 0.12, act ? 0.42 : 0.16);
        cairo_rectangle(cr, x + 1, 2, tw - 2, bar_h - 2);
        cairo_fill(cr);
        if (act) {
            cairo_set_source_rgb(cr, 1.0, 0.84, 0.0);
            cairo_rectangle(cr, x + 1, bar_h - 2, tw - 2, 2);
            cairo_fill(cr);
        }
        double g = act ? 0.95 : 0.62;
        text(cr, "Sans 10", base_name(ts->tabs[i].s->filepath), x + 8, 2,
             tw - 8 - CLOSE_W, g, g, g + 0.03, 0, bar_h - 2);
        text(cr, "Sans Bold 11", "×", x + tw - CLOSE_W, 2, CLOSE_W - 2, g, g, g, 1, bar_h - 2);
    }
    text(cr, "Sans Bold 13", "+", ts->n * tw, 0, PLUS_W, 0.7, 0.75, 0.85, 1, bar_h);
}

TabHit tabs_hit_test(const TabSet *ts, int w, int bar_h, int x, int y) {
    TabHit h = {TAB_HIT_NONE, -1};
    if (y < 0 || y >= bar_h) return h;
    double tw = tab_width(ts, w);
    int i = (int)(x / tw);
    if (i >= 0 && i < ts->n) {
        h.index = i;
        h.kind = (x - i * tw >= tw - CLOSE_W) ? TAB_HIT_CLOSE : TAB_HIT_TAB;
    } else if (x >= ts->n * tw && x < ts->n * tw + PLUS_W) {
        h.kind = TAB_HIT_PLUS;
    }
    return h;
}
