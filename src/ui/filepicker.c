#include "filepicker.h"
#include <dirent.h>
#include <pango/pangocairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#define ROW_H    30
#define FOOTER_H 56
#define MARGIN   40

/* ---------- utilidades ---------- */

static int has_md_ext(const char *n) {
    size_t l = strlen(n);
    if (l > 3 && strcasecmp(n + l - 3, ".md") == 0) return 1;
    if (l > 9 && strcasecmp(n + l - 9, ".markdown") == 0) return 1;
    return 0;
}

static int cmp_entries(const void *a, const void *b) {
    const FPEntry *x = a, *y = b;
    if (x->is_dir != y->is_dir) return y->is_dir - x->is_dir;
    return strcasecmp(x->name, y->name);
}

static void free_entries(FilePicker *fp) {
    for (int i = 0; i < fp->n_entries; i++) free(fp->entries[i].name);
    free(fp->entries);
    free(fp->visible);
    fp->entries = NULL;
    fp->visible = NULL;
    fp->n_entries = fp->n_visible = 0;
}

/* Subcadena sin distinguir mayúsculas (ASCII) */
static int contains_ci(const char *h, const char *n) {
    size_t ln = strlen(n);
    if (!ln) return 1;
    for (; *h; h++)
        if (strncasecmp(h, n, ln) == 0) return 1;
    return 0;
}

static void apply_filter(FilePicker *fp) {
    free(fp->visible);
    fp->visible = malloc(sizeof(int) * (fp->n_entries > 0 ? fp->n_entries : 1));
    fp->n_visible = 0;
    for (int i = 0; i < fp->n_entries; i++) {
        const char *n = fp->entries[i].name;
        if (strcmp(n, "..") == 0) {
            if (fp->filter[0]) continue;
        } else if (!contains_ci(n, fp->filter)) {
            continue;
        }
        fp->visible[fp->n_visible++] = i;
    }
    fp->selected = 0;
    fp->scroll = 0;
}

static int read_dir(FilePicker *fp, const char *path) {
    DIR *d = opendir(path);
    if (!d) {
        snprintf(fp->status, sizeof(fp->status), "No se puede abrir: %s", path);
        return -1;
    }
    free_entries(fp);
    int cap = 64;
    fp->entries = malloc(sizeof(FPEntry) * cap);
    struct dirent *de;
    while ((de = readdir(d))) {
        const char *n = de->d_name;
        if (strcmp(n, ".") == 0 || strcmp(n, "..") == 0) continue;
        if (n[0] == '.' && !fp->show_hidden) continue;
        char full[PATH_MAX + 300];
        snprintf(full, sizeof(full), "%s/%s", path, n);
        struct stat st;
        if (stat(full, &st) != 0) continue;
        int is_dir = S_ISDIR(st.st_mode);
        if (!is_dir && !(S_ISREG(st.st_mode) && has_md_ext(n))) continue;
        if (fp->n_entries == cap) {
            cap *= 2;
            fp->entries = realloc(fp->entries, sizeof(FPEntry) * cap);
        }
        fp->entries[fp->n_entries].name = strdup(n);
        fp->entries[fp->n_entries].is_dir = is_dir;
        fp->n_entries++;
    }
    closedir(d);
    qsort(fp->entries, fp->n_entries, sizeof(FPEntry), cmp_entries);
    /* ".." siempre primero (salvo en la raíz) */
    if (strcmp(path, "/") != 0) {
        fp->entries = realloc(fp->entries, sizeof(FPEntry) * (fp->n_entries + 1));
        memmove(fp->entries + 1, fp->entries, sizeof(FPEntry) * fp->n_entries);
        fp->entries[0].name = strdup("..");
        fp->entries[0].is_dir = 1;
        fp->n_entries++;
    }
    snprintf(fp->dir, sizeof(fp->dir), "%s", path);
    fp->filter[0] = '\0';
    apply_filter(fp);
    return 0;
}

/* Navega a un directorio; si 'select_name' no es NULL, lo deja seleccionado */
static void goto_dir(FilePicker *fp, const char *path, const char *select_name) {
    char real[PATH_MAX];
    if (!realpath(path, real)) {
        snprintf(fp->status, sizeof(fp->status), "Ruta inválida: %s", path);
        return;
    }
    if (read_dir(fp, real) != 0) return;
    fp->status[0] = '\0';
    if (select_name) {
        for (int i = 0; i < fp->n_visible; i++)
            if (strcmp(fp->entries[fp->visible[i]].name, select_name) == 0) {
                fp->selected = i;
                break;
            }
    }
}

/* ---------- API ---------- */

void fp_init(FilePicker *fp, const char *start_dir) {
    memset(fp, 0, sizeof(*fp));
    char cwd[PATH_MAX];
    const char *dir = start_dir;
    if (!dir || !*dir) dir = getcwd(cwd, sizeof(cwd)) ? cwd : "/";
    char real[PATH_MAX];
    if (!realpath(dir, real)) snprintf(real, sizeof(real), "/");
    if (read_dir(fp, real) != 0) read_dir(fp, "/");
    fp->status[0] = '\0';
    fp->row_h = ROW_H;
}

void fp_free(FilePicker *fp) { free_entries(fp); }

void fp_set_status(FilePicker *fp, const char *msg) {
    snprintf(fp->status, sizeof(fp->status), "%s", msg ? msg : "");
}

const char *fp_chosen_path(FilePicker *fp) { return fp->chosen; }

static void ensure_visible(FilePicker *fp) {
    int rows = fp->rows > 0 ? fp->rows : 10;
    if (fp->selected < fp->scroll) fp->scroll = fp->selected;
    if (fp->selected >= fp->scroll + rows) fp->scroll = fp->selected - rows + 1;
    int max_scroll = fp->n_visible - rows;
    if (max_scroll < 0) max_scroll = 0;
    if (fp->scroll > max_scroll) fp->scroll = max_scroll;
    if (fp->scroll < 0) fp->scroll = 0;
}

static void go_parent(FilePicker *fp) {
    char base[256];
    const char *slash = strrchr(fp->dir, '/');
    snprintf(base, sizeof(base), "%s", slash ? slash + 1 : "");
    char parent[PATH_MAX + 4];
    snprintf(parent, sizeof(parent), "%s/..", fp->dir);
    goto_dir(fp, parent, base);
    ensure_visible(fp);
}

static FPResult activate(FilePicker *fp) {
    if (fp->n_visible == 0) return FP_NONE;
    FPEntry *e = &fp->entries[fp->visible[fp->selected]];
    if (strcmp(e->name, "..") == 0) {
        go_parent(fp);
        return FP_REDRAW;
    }
    char full[PATH_MAX];
    if (snprintf(full, sizeof(full), "%s/%s", fp->dir, e->name) >= (int)sizeof(full)) {
        fp_set_status(fp, "Ruta demasiado larga");
        return FP_REDRAW;
    }
    if (e->is_dir) {
        goto_dir(fp, full, NULL);
        return FP_REDRAW;
    }
    snprintf(fp->chosen, sizeof(fp->chosen), "%s", full);
    return FP_CHOSEN;
}

static void utf8_pop(char *s) {
    size_t l = strlen(s);
    if (!l) return;
    l--;
    while (l > 0 && ((unsigned char)s[l] & 0xC0) == 0x80) l--;
    s[l] = '\0';
}

FPResult fp_key(FilePicker *fp, FPKey key, const char *text) {
    int rows = fp->rows > 0 ? fp->rows : 10;
    switch (key) {
    case FPK_UP:       if (fp->selected > 0) fp->selected--; break;
    case FPK_DOWN:     if (fp->selected < fp->n_visible - 1) fp->selected++; break;
    case FPK_PAGEUP:   fp->selected -= rows; if (fp->selected < 0) fp->selected = 0; break;
    case FPK_PAGEDOWN: fp->selected += rows;
                       if (fp->selected > fp->n_visible - 1) fp->selected = fp->n_visible - 1;
                       if (fp->selected < 0) fp->selected = 0;
                       break;
    case FPK_HOME:     fp->selected = 0; break;
    case FPK_END:      fp->selected = fp->n_visible > 0 ? fp->n_visible - 1 : 0; break;
    case FPK_ENTER: {
        fp->status[0] = '\0';
        FPResult r = activate(fp);
        ensure_visible(fp);
        return r;
    }
    case FPK_BACKSPACE:
        if (fp->filter[0]) { utf8_pop(fp->filter); apply_filter(fp); }
        else go_parent(fp);
        return FP_REDRAW;
    case FPK_ESCAPE:
        if (fp->filter[0]) { fp->filter[0] = '\0'; apply_filter(fp); return FP_REDRAW; }
        return FP_CANCEL;
    case FPK_TOGGLE_HIDDEN: {
        fp->show_hidden = !fp->show_hidden;
        char d[PATH_MAX];
        snprintf(d, sizeof(d), "%s", fp->dir);
        read_dir(fp, d);
        ensure_visible(fp);
        return FP_REDRAW;
    }
    case FPK_TEXT:
        if (text && (unsigned char)text[0] >= 0x20 && text[0] != 0x7f) {
            size_t l = strlen(fp->filter), t = strlen(text);
            if (l + t < sizeof(fp->filter)) {
                memcpy(fp->filter + l, text, t + 1);
                apply_filter(fp);
            }
            return FP_REDRAW;
        }
        return FP_NONE;
    }
    ensure_visible(fp);
    return FP_REDRAW;
}

FPResult fp_scroll(FilePicker *fp, int lines) {
    int rows = fp->rows > 0 ? fp->rows : 10;
    int max_scroll = fp->n_visible - rows;
    if (max_scroll < 0) max_scroll = 0;
    int ns = fp->scroll + lines;
    if (ns < 0) ns = 0;
    if (ns > max_scroll) ns = max_scroll;
    if (ns == fp->scroll) return FP_NONE;
    fp->scroll = ns;
    return FP_REDRAW;
}

FPResult fp_click(FilePicker *fp, int x, int y, int is_double) {
    (void)x;
    if (fp->row_h <= 0 || y < fp->list_y) return FP_NONE;
    int row = (y - fp->list_y) / fp->row_h;
    if (row >= fp->rows) return FP_NONE;
    int idx = fp->scroll + row;
    if (idx >= fp->n_visible) return FP_NONE;
    fp->selected = idx;
    if (is_double) return activate(fp);
    return FP_REDRAW;
}

/* ---------- dibujo ---------- */

static void draw_text(cairo_t *cr, const char *font, const char *txt, double x,
                      double y, double max_w, PangoEllipsizeMode ell, double r,
                      double g, double b) {
    PangoLayout *l = pango_cairo_create_layout(cr);
    PangoFontDescription *fd = pango_font_description_from_string(font);
    pango_layout_set_font_description(l, fd);
    pango_layout_set_text(l, txt, -1);
    if (max_w > 0) {
        pango_layout_set_width(l, (int)(max_w * PANGO_SCALE));
        pango_layout_set_ellipsize(l, ell);
    }
    cairo_set_source_rgb(cr, r, g, b);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, l);
    pango_font_description_free(fd);
    g_object_unref(l);
}

void fp_render(FilePicker *fp, cairo_t *cr, int w, int h) {
    fp->last_w = w;
    fp->last_h = h;
    cairo_set_source_rgb(cr, 0.07, 0.07, 0.10);
    cairo_paint(cr);

    double cw = w - 2.0 * MARGIN;
    if (cw < 100) cw = 100;
    const PangoEllipsizeMode END = PANGO_ELLIPSIZE_END;

    draw_text(cr, "Sans Bold 22", "Abrir presentación", MARGIN, 18, cw, END,
              0.93, 0.93, 0.96);
    draw_text(cr, "Monospace 11", fp->dir, MARGIN, 54, cw, PANGO_ELLIPSIZE_START,
              0.55, 0.60, 0.75);

    /* caja de filtro */
    double fy = 80;
    cairo_set_source_rgb(cr, 0.12, 0.13, 0.18);
    cairo_rectangle(cr, MARGIN, fy, cw, 30);
    cairo_fill(cr);
    if (fp->filter[0])
        draw_text(cr, "Sans 12", fp->filter, MARGIN + 10, fy + 5, cw - 20, END,
                  0.95, 0.95, 1);
    else
        draw_text(cr, "Sans Italic 12", "Escribe para filtrar…", MARGIN + 10,
                  fy + 5, cw - 20, END, 0.40, 0.42, 0.52);

    int list_y = (int)fy + 30 + 12;
    int rows = (h - list_y - FOOTER_H) / ROW_H;
    if (rows < 1) rows = 1;
    fp->rows = rows;
    fp->list_y = list_y;
    fp->row_h = ROW_H;
    ensure_visible(fp);

    if (fp->n_visible == 0)
        draw_text(cr, "Sans Italic 12", "(sin archivos .md ni carpetas)",
                  MARGIN + 10, list_y + 6, cw, END, 0.45, 0.47, 0.57);

    for (int r = 0; r < rows; r++) {
        int idx = fp->scroll + r;
        if (idx >= fp->n_visible) break;
        FPEntry *e = &fp->entries[fp->visible[idx]];
        double y = list_y + r * ROW_H;
        if (idx == fp->selected) {
            cairo_set_source_rgb(cr, 0.20, 0.25, 0.42);
            cairo_rectangle(cr, MARGIN, y, cw, ROW_H - 2);
            cairo_fill(cr);
        }
        if (e->is_dir)
            draw_text(cr, "Sans Bold 12", "▸", MARGIN + 10, y + 4, 20, END,
                      0.95, 0.80, 0.35);
        else
            draw_text(cr, "Sans 12", "•", MARGIN + 10, y + 4, 20, END,
                      0.55, 0.80, 0.95);
        char label[600];
        if (strcmp(e->name, "..") == 0)
            snprintf(label, sizeof(label), ".. (subir)");
        else if (e->is_dir)
            snprintf(label, sizeof(label), "%s/", e->name);
        else
            snprintf(label, sizeof(label), "%s", e->name);
        if (e->is_dir)
            draw_text(cr, "Sans Bold 12", label, MARGIN + 36, y + 4, cw - 50,
                      END, 0.95, 0.88, 0.65);
        else
            draw_text(cr, "Sans 12", label, MARGIN + 36, y + 4, cw - 50, END,
                      0.88, 0.90, 0.95);
    }
    /* barra de desplazamiento */
    if (fp->n_visible > rows) {
        double th = (double)rows * ROW_H;
        double bh = th * rows / fp->n_visible;
        double by = list_y + th * fp->scroll / fp->n_visible;
        cairo_set_source_rgb(cr, 0.3, 0.32, 0.4);
        cairo_rectangle(cr, w - MARGIN + 8, by, 4, bh);
        cairo_fill(cr);
    }

    /* pie */
    if (fp->status[0])
        draw_text(cr, "Sans 11", fp->status, MARGIN, h - 48, cw, END, 1.0, 0.45, 0.45);
    draw_text(cr, "Sans 10",
              "↑↓ mover · Enter abrir · Backspace subir/borrar filtro · Ctrl+H ocultos · Esc cancelar",
              MARGIN, h - 26, cw, END, 0.50, 0.53, 0.65);
}
