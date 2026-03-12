// slides.c — Presentador de slides desde archivo markdown
// Compilar:
//   gcc slides.c -o slides \
//       $(pkg-config --cflags --libs x11 cairo pangocairo) -lm
//
// Uso:
//   ./slides presentacion.md
//
// Formato del archivo:
//   ---          → separador de slides
//   # Título     → título grande
//   ## Subtítulo → subtítulo
//   Texto        → párrafo normal
//   - Item       → viñeta (soporta dos niveles con "  - ")
//   !imagen.png  → imagen PNG centrada
//   | A | B | C |  → tabla (segunda fila con --- es el header)
//   (línea vacía → espacio vertical)
//
// Controles:
//   → / Enter / Click derecho  → siguiente slide
//   ← / Backspace / Click izq  → slide anterior
//   F                          → fullscreen toggle
//   Q / ESC                    → salir

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cairo/cairo-xlib.h>
#include <cairo/cairo.h>
#include <math.h>
#include <pango/pangocairo.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// ── Configuración ─────────────────────────────────────────────────────────────

#define WIN_W         1280
#define WIN_H         720
#define MARGIN_X      80.0
#define MARGIN_Y      60.0
#define MAX_SLIDES    256
#define MAX_LINES     128    // líneas por slide
#define MAX_LINE_LEN  1024
#define MAX_IMG_CACHE 64

// Paleta — dark blue theme coherente con los otros programas
#define COL_BG_R      0.04
#define COL_BG_G      0.06
#define COL_BG_B      0.18

#define COL_TITLE_R   1.00
#define COL_TITLE_G   1.00
#define COL_TITLE_B   1.00

#define COL_SUB_R     0.65
#define COL_SUB_G     0.78
#define COL_SUB_B     1.00

#define COL_BODY_R    0.88
#define COL_BODY_G    0.90
#define COL_BODY_B    0.96

#define COL_BULLET_R  0.98
#define COL_BULLET_G  0.75
#define COL_BULLET_B  0.20

#define COL_TABLE_HDR_R  0.10
#define COL_TABLE_HDR_G  0.18
#define COL_TABLE_HDR_B  0.45

#define COL_TABLE_ROW_R  0.06
#define COL_TABLE_ROW_G  0.09
#define COL_TABLE_ROW_B  0.25

#define COL_TABLE_ALT_R  0.08
#define COL_TABLE_ALT_G  0.12
#define COL_TABLE_ALT_B  0.32

#define COL_TABLE_BDR_R  0.20
#define COL_TABLE_BDR_G  0.28
#define COL_TABLE_BDR_B  0.55

#define COL_ACCENT_R  0.25
#define COL_ACCENT_G  0.55
#define COL_ACCENT_B  1.00

#define COL_NUM_R     0.40
#define COL_NUM_G     0.50
#define COL_NUM_B     0.75

// ── Definir COL_LABEL para el placeholder ────────────────────────────────────
// (necesario porque COL_LABEL se define como triple macro)
#define COL_LABEL_R 0.55
#define COL_LABEL_G 0.65
#define COL_LABEL_B 0.90

// ── Tipos ─────────────────────────────────────────────────────────────────────

typedef enum {
    LINE_EMPTY,
    LINE_TITLE,       // # texto
    LINE_SUBTITLE,    // ## texto
    LINE_BODY,        // texto normal
    LINE_BULLET1,     // - texto
    LINE_BULLET2,     //   - texto (subviñeta)
    LINE_IMAGE,       // !ruta.png
    LINE_TABLE_ROW,   // | col | col | ...
    LINE_TABLE_SEP,   // | --- | --- | ... (separador de header)
} LineType;

typedef struct {
    LineType type;
    char     text[MAX_LINE_LEN];  // contenido sin prefijo
    // Para tablas: columnas parseadas
    char     cols[16][256];
    int      ncols;
} SlideLine;

typedef struct {
    SlideLine lines[MAX_LINES];
    int       nlines;
} Slide;

// Cache de imágenes para no recargar en cada frame
typedef struct {
    char             path[512];
    cairo_surface_t *surface;
} ImgCache;

static ImgCache img_cache[MAX_IMG_CACHE];
static int      img_cache_count = 0;

// ── Parser de líneas ──────────────────────────────────────────────────────────

// Trim in-place (modifica el buffer)
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' ||
                       *end == '\r' || *end == '\n'))
        *end-- = '\0';
    return s;
}

// Parsea "| A | B | C |" → cols[] y devuelve ncols
static int parse_table_row(const char *line, char cols[][256], int max_cols) {
    int n = 0;
    const char *p = line;
    while (*p && n < max_cols) {
        // Avanzar hasta '|'
        while (*p && *p != '|') p++;
        if (!*p) break;
        p++; // saltar '|'
        // Leer hasta el siguiente '|'
        const char *start = p;
        while (*p && *p != '|') p++;
        // Copiar y trim
        int len = (int)(p - start);
        if (len >= 255) len = 255;
        strncpy(cols[n], start, len);
        cols[n][len] = '\0';
        char *t = trim(cols[n]);
        if (t != cols[n]) memmove(cols[n], t, strlen(t) + 1);
        // Ignorar columnas vacías al inicio/final (artefactos del '|' externo)
        if (strlen(cols[n]) > 0) n++;
    }
    return n;
}

// ¿Es línea separadora de tabla? (solo -, :, |, espacio)
static int is_table_sep(const char *s) {
    for (; *s; s++)
        if (*s != '-' && *s != ':' && *s != '|' && *s != ' ' && *s != '\t')
            return 0;
    return strchr(s - strlen(s), '-') != NULL || strlen(s) == 0
           ? (int)strspn(s - strlen(s), "-:| \t") == (int)strlen(s - strlen(s))
           : 0;
}

static int is_table_sep2(const char *line) {
    // Toda la línea compuesta de |, -, :, espacio
    const char *p = line;
    int has_dash = 0;
    for (; *p; p++) {
        if (*p == '-') has_dash = 1;
        else if (*p != '|' && *p != ':' && *p != ' ' && *p != '\t')
            return 0;
    }
    return has_dash;
}

static void parse_line(const char *raw, SlideLine *out) {
    char buf[MAX_LINE_LEN];
    strncpy(buf, raw, MAX_LINE_LEN - 1);
    buf[MAX_LINE_LEN - 1] = '\0';
    // Quitar \r\n
    char *p = buf + strlen(buf) - 1;
    while (p >= buf && (*p == '\r' || *p == '\n')) *p-- = '\0';

    out->ncols = 0;
    memset(out->cols, 0, sizeof(out->cols));

    // Línea vacía
    char *s = buf;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') { out->type = LINE_EMPTY; out->text[0] = '\0'; return; }

    // Título ##
    if (strncmp(s, "## ", 3) == 0) {
        out->type = LINE_SUBTITLE;
        strncpy(out->text, s + 3, MAX_LINE_LEN - 1);
        return;
    }
    // Título #
    if (strncmp(s, "# ", 2) == 0) {
        out->type = LINE_TITLE;
        strncpy(out->text, s + 2, MAX_LINE_LEN - 1);
        return;
    }
    // Imagen
    if (s[0] == '!') {
        out->type = LINE_IMAGE;
        strncpy(out->text, s + 1, MAX_LINE_LEN - 1);
        // Quitar posible '[]()' de markdown si alguien usa ![alt](ruta)
        // Soportamos tanto !ruta.png como ![alt](ruta.png)
        if (s[1] == '[') {
            char *paren = strchr(s, '(');
            char *close = strchr(s, ')');
            if (paren && close && close > paren) {
                int len = (int)(close - paren - 1);
                strncpy(out->text, paren + 1, len);
                out->text[len] = '\0';
            }
        }
        return;
    }
    // Tabla separadora
    if (s[0] == '|' && is_table_sep2(s)) {
        out->type = LINE_TABLE_SEP;
        out->text[0] = '\0';
        return;
    }
    // Tabla fila
    if (s[0] == '|') {
        out->type = LINE_TABLE_ROW;
        strncpy(out->text, s, MAX_LINE_LEN - 1);
        out->ncols = parse_table_row(s, out->cols, 16);
        return;
    }
    // Subviñeta (dos espacios + "- ")
    if ((strncmp(s, "  - ", 4) == 0 || strncmp(buf, "  - ", 4) == 0)) {
        out->type = LINE_BULLET2;
        const char *content = strstr(buf, "- ") + 2;
        strncpy(out->text, content, MAX_LINE_LEN - 1);
        return;
    }
    // Viñeta
    if (strncmp(s, "- ", 2) == 0) {
        out->type = LINE_BULLET1;
        strncpy(out->text, s + 2, MAX_LINE_LEN - 1);
        return;
    }
    // Párrafo normal
    out->type = LINE_BODY;
    strncpy(out->text, s, MAX_LINE_LEN - 1);
}

// ── Carga del archivo ─────────────────────────────────────────────────────────

static int load_slides(const char *path, Slide *slides, int max_slides) {
    FILE *fp = fopen(path, "r");
    if (!fp) { fprintf(stderr, "No se pudo abrir: %s\n", path); return 0; }

    int n = 0;
    memset(&slides[0], 0, sizeof(Slide));
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp) && n < max_slides) {
        // Separador de slide
        char trimmed[MAX_LINE_LEN];
        strncpy(trimmed, line, MAX_LINE_LEN - 1);
        char *t = trim(trimmed);
        if (strcmp(t, "---") == 0) {
            n++;
            if (n < max_slides)
                memset(&slides[n], 0, sizeof(Slide));
            continue;
        }
        Slide *cur = &slides[n];
        if (cur->nlines < MAX_LINES) {
            parse_line(line, &cur->lines[cur->nlines]);
            cur->nlines++;
        }
    }
    fclose(fp);
    return n + 1;
}

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

// Convierte markdown inline (**bold**, *italic*, __bold__, _italic_)
// a Pango markup. También escapa &, <, > para que Pango no los confunda.
// out debe tener espacio suficiente (use MAX_LINE_LEN * 4 para seguridad).
static void md_to_markup(const char *in, char *out, size_t out_size) {
    size_t wi = 0;  // write index
    const char *p = in;

#define WRITE(s) do { \
    size_t _l = strlen(s); \
    if (wi + _l < out_size - 1) { memcpy(out + wi, s, _l); wi += _l; } \
} while(0)

    while (*p) {
        // Escapar caracteres especiales de Pango XML
        if (*p == '&') { WRITE("&amp;");  p++; continue; }
        if (*p == '<') { WRITE("&lt;");   p++; continue; }
        if (*p == '>') { WRITE("&gt;");   p++; continue; }

        // **bold** o __bold__
        if ((p[0] == '*' && p[1] == '*') ||
            (p[0] == '_' && p[1] == '_')) {
            char delim[3] = { p[0], p[1], '\0' };
            const char *end = strstr(p + 2, delim);
            if (end) {
                WRITE("<b>");
                // Copiar contenido interior recursivamente sería complejo;
                // por simplicidad lo copiamos escapando solo & < >
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

        // *italic* (un solo asterisco, que no sea **)
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

        // _italic_ (un solo guion bajo, que no sea __)
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

// Dibuja texto con Pango (con soporte de bold/italic via markup)
// y devuelve la altura ocupada.
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

// ── Render de tabla ───────────────────────────────────────────────────────────

// Devuelve la altura total de la tabla dibujada
static double render_table(cairo_t *cr, PangoLayout *lay_body,
                            SlideLine *lines, int start, int count,
                            double x, double y, double max_w) {
    if (count == 0) return 0.0;

    // Encontrar el número máximo de columnas
    int max_cols = 0;
    int header_row = -1;  // índice de la fila SEP (la que sigue al header)
    for (int i = 0; i < count; i++) {
        if (lines[start + i].type == LINE_TABLE_ROW &&
            lines[start + i].ncols > max_cols)
            max_cols = lines[start + i].ncols;
        if (lines[start + i].type == LINE_TABLE_SEP)
            header_row = i; // la fila 0 es el header si hay SEP en fila 1
    }
    if (max_cols == 0) return 0.0;

    double col_w   = (max_w - 2.0) / max_cols;
    double row_h   = 40.0;
    double cur_y   = y;
    int    data_row = 0;  // contador de filas de datos (para alternado)

    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_TABLE_SEP) continue;
        if (sl->type != LINE_TABLE_ROW) continue;

        int is_header = (header_row > 0 && i == 0);

        // Fondo de fila
        if (is_header)
            set_color(cr, COL_TABLE_HDR_R, COL_TABLE_HDR_G, COL_TABLE_HDR_B);
        else if (data_row % 2 == 0)
            set_color(cr, COL_TABLE_ROW_R, COL_TABLE_ROW_G, COL_TABLE_ROW_B);
        else
            set_color(cr, COL_TABLE_ALT_R, COL_TABLE_ALT_G, COL_TABLE_ALT_B);

        cairo_rectangle(cr, x, cur_y, max_w, row_h);
        cairo_fill(cr);

        // Bordes de celda
        set_color(cr, COL_TABLE_BDR_R, COL_TABLE_BDR_G, COL_TABLE_BDR_B);
        cairo_set_line_width(cr, 0.5);
        for (int c = 0; c <= max_cols; c++) {
            double cx = x + c * col_w;
            cairo_move_to(cr, cx, cur_y);
            cairo_line_to(cr, cx, cur_y + row_h);
            cairo_stroke(cr);
        }
        // Borde horizontal
        cairo_move_to(cr, x,          cur_y + row_h);
        cairo_line_to(cr, x + max_w,  cur_y + row_h);
        cairo_stroke(cr);

        // Texto de celdas
        if (is_header)
            set_color(cr, COL_BULLET_R, COL_BULLET_G, COL_BULLET_B);
        else
            set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);

        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            pango_layout_set_width(lay_body,
                                   (int)((col_w - 12.0) * PANGO_SCALE));
            // Medir la altura real del texto para centrarlo verticalmente
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            double text_x = x + c * col_w + 6.0;
            double text_y = cur_y + (row_h - (double)th) / 2.0;
            cairo_move_to(cr, text_x, text_y);
            pango_cairo_show_layout(cr, lay_body);
        }

        cur_y += row_h;
        if (!is_header) data_row++;
    }

    // Borde superior y exterior
    set_color(cr, COL_TABLE_BDR_R, COL_TABLE_BDR_G, COL_TABLE_BDR_B);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, x, y, max_w, cur_y - y);
    cairo_stroke(cr);

    // Restaurar ancho del layout
    pango_layout_set_width(lay_body,
        (int)((max_w) * PANGO_SCALE));

    return cur_y - y;
}

// ── Render de un slide completo ───────────────────────────────────────────────

static void render_slide(cairo_t *cr, const Slide *slide,
                         int index, int total,
                         int win_w, int win_h) {
    double content_w = win_w - MARGIN_X * 2.0;

    // Layouts — se crean y destruyen por frame (simple y correcto)
    PangoLayout *lay_title    = make_layout(cr, "Inter Bold 44",    content_w);
    PangoLayout *lay_subtitle = make_layout(cr, "Inter 26",         content_w);
    PangoLayout *lay_body     = make_layout(cr, "Inter 20",         content_w);
    PangoLayout *lay_bullet   = make_layout(cr, "Inter 20",         content_w - 30);
    PangoLayout *lay_bullet2  = make_layout(cr, "Inter 18",         content_w - 60);
    PangoLayout *lay_num      = make_layout(cr, "Inter 13",         200);

    double y = MARGIN_Y;

    // Agrupar líneas de tabla contiguas
    int i = 0;
    while (i < slide->nlines) {
        const SlideLine *sl = &slide->lines[i];

        switch (sl->type) {

        case LINE_EMPTY:
            y += 12.0;
            i++;
            break;

        case LINE_TITLE:
            set_color(cr, COL_TITLE_R, COL_TITLE_G, COL_TITLE_B);
            y += render_pango(cr, lay_title, sl->text, MARGIN_X, y);

            // Línea decorativa bajo el título
            set_color(cr, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B);
            cairo_set_line_width(cr, 2.5);
            cairo_move_to(cr, MARGIN_X,              y + 8);
            cairo_line_to(cr, MARGIN_X + content_w,  y + 8);
            cairo_stroke(cr);
            y += 22.0;
            i++;
            break;

        case LINE_SUBTITLE:
            set_color(cr, COL_SUB_R, COL_SUB_G, COL_SUB_B);
            y += render_pango(cr, lay_subtitle, sl->text, MARGIN_X, y);
            y += 14.0;
            i++;
            break;

        case LINE_BODY:
            set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);
            y += render_pango(cr, lay_body, sl->text, MARGIN_X, y);
            y += 8.0;
            i++;
            break;

        case LINE_BULLET1: {
            // Punto de viñeta
            set_color(cr, COL_BULLET_R, COL_BULLET_G, COL_BULLET_B);
            cairo_arc(cr, MARGIN_X + 8, y + 11, 4, 0, 2 * M_PI);
            cairo_fill(cr);
            set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);
            double h = render_pango(cr, lay_bullet, sl->text,
                                    MARGIN_X + 22.0, y);
            y += h + 6.0;
            i++;
            break;
        }

        case LINE_BULLET2: {
            // Rombo para subviñeta
            double bx = MARGIN_X + 38, by = y + 10;
            set_color(cr, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B);
            cairo_move_to(cr, bx,     by - 4);
            cairo_line_to(cr, bx + 4, by);
            cairo_line_to(cr, bx,     by + 4);
            cairo_line_to(cr, bx - 4, by);
            cairo_close_path(cr);
            cairo_fill(cr);
            set_color(cr, COL_BODY_R, COL_BODY_G, COL_BODY_B);
            double h = render_pango(cr, lay_bullet2, sl->text,
                                    MARGIN_X + 52.0, y);
            y += h + 4.0;
            i++;
            break;
        }

        case LINE_IMAGE: {
            cairo_surface_t *img = get_image(sl->text);
            if (img) {
                int iw = cairo_image_surface_get_width(img);
                int ih = cairo_image_surface_get_height(img);
                // Escalar para que quepa en el área disponible
                double avail_h = win_h - y - MARGIN_Y - 40.0;
                double avail_w = content_w;
                double scale = 1.0;
                if (iw > avail_w) scale = avail_w / iw;
                if (ih * scale > avail_h) scale = avail_h / ih;
                double dw = iw * scale, dh = ih * scale;
                double dx = MARGIN_X + (content_w - dw) / 2.0;

                cairo_save(cr);
                cairo_translate(cr, dx, y);
                cairo_scale(cr, scale, scale);
                cairo_set_source_surface(cr, img, 0, 0);
                cairo_paint(cr);
                cairo_restore(cr);
                y += dh + 14.0;
            } else {
                // Placeholder si la imagen no carga
                set_color(cr, 0.15, 0.18, 0.38);
                cairo_rectangle(cr, MARGIN_X, y, content_w, 80);
                cairo_fill(cr);
                set_color(cr, COL_LABEL_R, COL_LABEL_G, COL_LABEL_B);
                char msg[MAX_LINE_LEN + 32];
                snprintf(msg, sizeof(msg), "⚠ No se encontró: %s", sl->text);
                render_pango(cr, lay_body, msg, MARGIN_X + 10, y + 28);
                y += 94.0;
            }
            i++;
            break;
        }

        case LINE_TABLE_ROW:
        case LINE_TABLE_SEP: {
            // Contar cuántas líneas de tabla hay seguidas
            int j = i;
            while (j < slide->nlines &&
                   (slide->lines[j].type == LINE_TABLE_ROW ||
                    slide->lines[j].type == LINE_TABLE_SEP))
                j++;
            int count = j - i;
            double th = render_table(cr, lay_body,
                                     (SlideLine *)slide->lines,
                                     i, count,
                                     MARGIN_X, y, content_w);
            y += th + 14.0;
            i = j;
            break;
        }

        default:
            i++;
            break;
        }
    }

    // ── Pie de página: número de slide ───────────────────────────────────────
    char num_buf[32];
    snprintf(num_buf, sizeof(num_buf), "%d / %d", index + 1, total);
    set_color(cr, COL_NUM_R, COL_NUM_G, COL_NUM_B);
    pango_layout_set_width(lay_num, 200 * PANGO_SCALE);
    pango_layout_set_alignment(lay_num, PANGO_ALIGN_RIGHT);
    render_pango(cr, lay_num, num_buf,
                 win_w - MARGIN_X - 200, win_h - 32.0);

    // Barra de progreso inferior
    double prog = (total > 1) ? (double)index / (total - 1) : 1.0;
    set_color(cr, 0.10, 0.14, 0.35);
    cairo_rectangle(cr, 0, win_h - 4, win_w, 4);
    cairo_fill(cr);
    set_color(cr, COL_ACCENT_R, COL_ACCENT_G, COL_ACCENT_B);
    cairo_rectangle(cr, 0, win_h - 4, win_w * prog, 4);
    cairo_fill(cr);

    // Limpiar layouts
    g_object_unref(lay_title);
    g_object_unref(lay_subtitle);
    g_object_unref(lay_body);
    g_object_unref(lay_bullet);
    g_object_unref(lay_bullet2);
    g_object_unref(lay_num);
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s presentacion.md\n", argv[0]);
        return 1;
    }

    static Slide slides[MAX_SLIDES];
    int n_slides = load_slides(argv[1], slides, MAX_SLIDES);
    if (n_slides == 0) return 1;
    fprintf(stderr, "[slides] %d slide(s) cargados desde %s\n",
            n_slides, argv[1]);

    // ── Ventana X11 ──────────────────────────────────────────────────────────
    Display *disp = XOpenDisplay(NULL);
    if (!disp) { fputs("No se pudo abrir display\n", stderr); return 1; }

    int scr = DefaultScreen(disp);
    int sw  = DisplayWidth(disp,  scr);
    int sh  = DisplayHeight(disp, scr);

    // Centrar la ventana
    int wx = (sw - WIN_W) / 2;
    int wy = (sh - WIN_H) / 2;
    Window win = XCreateSimpleWindow(disp, RootWindow(disp, scr),
                                     wx, wy, WIN_W, WIN_H,
                                     0, 0, 0);

    XStoreName(disp, win, argv[1]);
    XSelectInput(disp, win,
                 KeyPressMask | ButtonPressMask | StructureNotifyMask);
    Atom wm_delete = XInternAtom(disp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(disp, win, &wm_delete, 1);

    // Átomo para fullscreen
    Atom wm_state  = XInternAtom(disp, "_NET_WM_STATE",            False);
    Atom fs_atom   = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);

    XMapWindow(disp, win);

    // ── Double buffer ────────────────────────────────────────────────────────
    int win_w = WIN_W, win_h = WIN_H;

    cairo_surface_t *sfc_screen = cairo_xlib_surface_create(
        disp, win, DefaultVisual(disp, scr), win_w, win_h);
    cairo_surface_t *sfc_back = cairo_surface_create_similar(
        sfc_screen, CAIRO_CONTENT_COLOR, win_w, win_h);
    cairo_t *cr      = cairo_create(sfc_back);
    cairo_t *cr_flip = cairo_create(sfc_screen);

    int current   = 0;
    int running   = 1;
    int dirty     = 1;   // 1 = hay que redibujar
    int fullscreen = 0;

    while (running) {
        // ── Eventos ──────────────────────────────────────────────────────────
        while (XPending(disp)) {
            XEvent ev;
            XNextEvent(disp, &ev);

            if (ev.type == KeyPress) {
                KeySym ks = XLookupKeysym(&ev.xkey, 0);
                if (ks == XK_q || ks == XK_Escape) {
                    running = 0;
                } else if (ks == XK_Right || ks == XK_Return ||
                           ks == XK_space || ks == XK_Next) {
                    if (current < n_slides - 1) { current++; dirty = 1; }
                } else if (ks == XK_Left || ks == XK_BackSpace ||
                           ks == XK_Prior) {
                    if (current > 0) { current--; dirty = 1; }
                } else if (ks == XK_Home) {
                    current = 0; dirty = 1;
                } else if (ks == XK_End) {
                    current = n_slides - 1; dirty = 1;
                } else if (ks == XK_f || ks == XK_F11) {
                    // Toggle fullscreen via EWMH
                    XEvent fev = {0};
                    fev.type = ClientMessage;
                    fev.xclient.window       = win;
                    fev.xclient.message_type = wm_state;
                    fev.xclient.format       = 32;
                    fev.xclient.data.l[0]    = fullscreen ? 0 : 1; // remove/add
                    fev.xclient.data.l[1]    = (long)fs_atom;
                    XSendEvent(disp, RootWindow(disp, scr), False,
                               SubstructureNotifyMask |
                               SubstructureRedirectMask, &fev);
                    fullscreen = !fullscreen;
                    dirty = 1;
                }
            }

            if (ev.type == ButtonPress) {
                if (ev.xbutton.button == Button1) {         // click izquierdo
                    if (current > 0) { current--; dirty = 1; }
                } else if (ev.xbutton.button == Button3) {  // click derecho
                    if (current < n_slides - 1) { current++; dirty = 1; }
                }
            }

            if (ev.type == ConfigureNotify) {
                int nw = ev.xconfigure.width;
                int nh = ev.xconfigure.height;
                if (nw != win_w || nh != win_h) {
                    win_w = nw; win_h = nh;
                    // Recrear superficies al cambiar tamaño
                    cairo_destroy(cr);
                    cairo_destroy(cr_flip);
                    cairo_surface_destroy(sfc_back);
                    cairo_surface_destroy(sfc_screen);

                    cairo_xlib_surface_set_size(sfc_screen, win_w, win_h);
                    sfc_screen = cairo_xlib_surface_create(
                        disp, win, DefaultVisual(disp, scr), win_w, win_h);
                    sfc_back = cairo_surface_create_similar(
                        sfc_screen, CAIRO_CONTENT_COLOR, win_w, win_h);
                    cr      = cairo_create(sfc_back);
                    cr_flip = cairo_create(sfc_screen);
                    dirty = 1;
                }
            }

            if (ev.type == ClientMessage)
                if ((Atom)ev.xclient.data.l[0] == wm_delete)
                    running = 0;
        }

        // ── Render (solo cuando hay cambio) ──────────────────────────────────
        if (dirty) {
            // Fondo
            cairo_set_source_rgb(cr, COL_BG_R, COL_BG_G, COL_BG_B);
            cairo_paint(cr);

            render_slide(cr, &slides[current], current, n_slides,
                         win_w, win_h);

            // Blit al screen
            cairo_set_source_surface(cr_flip, sfc_back, 0, 0);
            cairo_paint(cr_flip);
            XFlush(disp);
            dirty = 0;
        }

        // Dormir un poco para no quemar CPU (no necesitamos animación aquí)
        usleep(8000);
    }

    // Cleanup
    for (int i = 0; i < img_cache_count; i++)
        cairo_surface_destroy(img_cache[i].surface);
    cairo_destroy(cr);
    cairo_destroy(cr_flip);
    cairo_surface_destroy(sfc_back);
    cairo_surface_destroy(sfc_screen);
    XCloseDisplay(disp);
    return 0;
}
