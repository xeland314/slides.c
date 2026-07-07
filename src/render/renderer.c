#include "../../slider.h"
#include "../core/internal.h"
#include "../core/highlighter.h"
#include <pango/pangocairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR // No necesitamos HDR flotante para slides simples
#include "../stb_image.h"

static ImgCache img_cache[MAX_IMG_CACHE];
static int      img_cache_count = 0;

// ── Cache de imágenes ─────────────────────────────────────────────────────────

// Convierte datos raw RGBA de stb_image a ARGB32 premultiplicado de Cairo
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
            
            if (channels == 1) { // Grey
                r = g = b = src[0];
            } else if (channels == 2) { // Grey + Alpha
                r = g = b = src[0];
                a = src[1];
            } else if (channels == 3) { // RGB
                r = src[0]; g = src[1]; b = src[2];
            } else if (channels == 4) { // RGBA
                r = src[0]; g = src[1]; b = src[2]; a = src[3];
            }

            // Pre-multiply alpha
            if (a != 255) {
                r = (r * a) / 255;
                g = (g * a) / 255;
                b = (b * a) / 255;
            }

            // Cairo usa formato nativo (ARBG o BGRA dependiendo de endianness), 
            // pero CAIRO_FORMAT_ARGB32 generalmente espera 0xAARRGGBB en little-endian (B G R A en memoria)
            // En x86 (little endian):
            // byte 0: B, byte 1: G, byte 2: R, byte 3: A
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

    // Attach user data to free memory when surface is destroyed
    static cairo_user_data_key_t key;
    cairo_surface_set_user_data(s, &key, surface_data, free);

    return s;
}

static ImgCache *get_image_cache(const char *path) {
    for (int i = 0; i < img_cache_count; i++)
        if (strcmp(img_cache[i].path, path) == 0)
            return &img_cache[i];

    if (img_cache_count >= MAX_IMG_CACHE) return NULL;

    ImgCache *cache = &img_cache[img_cache_count];
    strncpy(cache->path, path, 511);
    
    // Detectar si es GIF por extensión (simple check)
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
        // stbi_load_gif_from_memory devuelve un buffer grande con todos los frames apilados
        unsigned char *data = stbi_load_gif_from_memory(buffer, fsize, &delays, &x, &y, &z, &comp, 4);
        free(buffer);

        if (!data) {
            fprintf(stderr, "Error cargando GIF: %s (%s)\n", path, stbi_failure_reason());
            return NULL;
        }

        cache->n_frames = z;
        cache->delays = delays; // stbi asigna esto, debemos liberarlo luego (o dejarlo en cache)
        cache->surfaces = malloc(sizeof(cairo_surface_t*) * z);
        cache->total_duration = 0;

        int frame_size = x * y * 4;
        for (int i = 0; i < z; i++) {
            cache->surfaces[i] = create_cairo_surface_from_stbi(data + i * frame_size, x, y, 4);
            cache->total_duration += delays[i];
        }
        
        // stbi_load_gif_from_memory devuelve un solo bloque para todos los datos, 
        // pero create_cairo_surface_from_stbi hace copias.
        stbi_image_free(data);

    } else {
        int w, h, c;
        unsigned char *data = stbi_load(path, &w, &h, &c, 4); // Forzar 4 canales
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

    img_cache_count++;
    return cache;
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

        // Inline Code (tiene prioridad y anula otros formatos dentro)
        if (*p == '`') {
            if (!code) { WRITE("<tt>"); code = 1; }
            else { WRITE("</tt>"); code = 0; }
            p++; continue;
        }

        if (code) {
            if (wi < out_size - 1) out[wi++] = *p;
            p++; continue;
        }

        // Triple (Bold + Italic)
        if (strncmp(p, "***", 3) == 0 || strncmp(p, "___", 3) == 0) {
            if (!bold && !italic) { WRITE("<b><i>"); bold = 1; italic = 1; }
            else if (bold && italic) { WRITE("</i></b>"); bold = 0; italic = 0; }
            p += 3; continue;
        }
        // Bold
        if (strncmp(p, "**", 2) == 0 || strncmp(p, "__", 2) == 0) {
            if (!bold) { WRITE("<b>"); bold = 1; }
            else { WRITE("</b>"); bold = 0; }
            p += 2; continue;
        }
        // Italic
        if (*p == '*' || *p == '_') {
            if (!italic) { WRITE("<i>"); italic = 1; }
            else { WRITE("</i>"); italic = 0; }
            p++; continue;
        }

        if (wi < out_size - 1) out[wi++] = *p;
        p++;
    }
    // Cerrar tags si quedaron abiertos por error en el MD
    if (italic) WRITE("</i>");
    if (bold) WRITE("</b>");
    
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
                            Slider *s, SlideLine *lines, int start, int count,
                            double x, double y, double max_w) {
    if (count == 0) return 0.0;
    int max_cols = 0, header_row = -1;
    for (int i = 0; i < count; i++) {
        if (lines[start + i].type == LINE_TABLE_ROW && lines[start + i].ncols > max_cols)
            max_cols = lines[start + i].ncols;
        if (lines[start + i].type == LINE_TABLE_SEP) header_row = i;
    }
    if (max_cols == 0) return 0.0;
    if (max_cols > 16) max_cols = 16;

    double col_w[16];
    double desired_w[16] = {0};
    double total_desired_w = 0.0;

    // First pass: measure preferred width for each column (without wrapping)
    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type != LINE_TABLE_ROW) continue;
        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            pango_layout_set_width(lay_body, -1); 
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            double w = (double)tw + 24.0 * s->font_scale; 
            if (w > desired_w[c]) desired_w[c] = w;
        }
    }

    for (int c = 0; c < max_cols; c++) {
        if (desired_w[c] < 40.0 * s->font_scale) desired_w[c] = 40.0 * s->font_scale;
        total_desired_w += desired_w[c];
    }

    // Proportional distribution (capped at max_w)
    for (int c = 0; c < max_cols; c++) {
        col_w[c] = (desired_w[c] / total_desired_w) * max_w;
    }

    double cur_y = y;
    int data_row = 0;

    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_TABLE_SEP) continue;
        if (sl->type != LINE_TABLE_ROW) continue;
        int is_header = (header_row >= 0 && i == 0); // i == 0 assume header if separator exists

        // Calculate height for this row based on wrapped content
        double row_h = 40.0 * s->font_scale; 
        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            pango_layout_set_width(lay_body, (int)((col_w[c] - 12.0 * s->font_scale) * PANGO_SCALE));
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            if ((double)th + 16.0 * s->font_scale > row_h) row_h = (double)th + 16.0 * s->font_scale;
        }

        // Fill background
        if (is_header) set_color(cr, s->theme->table_hdr_r, s->theme->table_hdr_g, s->theme->table_hdr_b);
        else if (data_row % 2 == 0) set_color(cr, s->theme->table_row_r, s->theme->table_row_g, s->theme->table_row_b);
        else set_color(cr, s->theme->table_alt_r, s->theme->table_alt_g, s->theme->table_alt_b);

        cairo_rectangle(cr, x, cur_y, max_w, row_h);
        cairo_fill(cr);

        // Borders
        set_color(cr, s->theme->table_bdr_r, s->theme->table_bdr_g, s->theme->table_bdr_b);
        cairo_set_line_width(cr, 0.5 * s->font_scale);
        
        // Vertical lines
        double cur_v_x = x;
        for (int c = 0; c <= max_cols; c++) {
            cairo_move_to(cr, cur_v_x, cur_y);
            cairo_line_to(cr, cur_v_x, cur_y + row_h);
            cairo_stroke(cr);
            if (c < max_cols) cur_v_x += col_w[c];
        }
        // Bottom line of row
        cairo_move_to(cr, x, cur_y + row_h);
        cairo_line_to(cr, x + max_w, cur_y + row_h);
        cairo_stroke(cr);

        // Content
        if (is_header) set_color(cr, s->theme->bullet_r, s->theme->bullet_g, s->theme->bullet_b);
        else set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);

        double cur_text_x = x;
        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            pango_layout_set_width(lay_body, (int)((col_w[c] - 12.0 * s->font_scale) * PANGO_SCALE));
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            cairo_move_to(cr, cur_text_x + 6.0 * s->font_scale, cur_y + (row_h - (double)th) / 2.0);
            pango_cairo_show_layout(cr, lay_body);
            cur_text_x += col_w[c];
        }
        cur_y += row_h;
        if (!is_header) data_row++;
    }
    set_color(cr, s->theme->table_bdr_r, s->theme->table_bdr_g, s->theme->table_bdr_b);
    cairo_set_line_width(cr, 1.0 * s->font_scale);
    cairo_rectangle(cr, x, y, max_w, cur_y - y);
    cairo_stroke(cr);
    pango_layout_set_width(lay_body, (int)((max_w) * PANGO_SCALE));
    return cur_y - y;
}

static double render_code_block(cairo_t *cr, PangoLayout *lay_code,
                               Slider *s, SlideLine *lines, int start, int count,
                               double x, double y, double max_w) {
    if (count <= 0) return 0.0;

    double lines_h_sum = 0.0;
    double *line_heights = malloc(count * sizeof(double));
    if (!line_heights) return 0.0;

    const char *lang = lines[start].text;

    for (int i = 0; i < count; i++) {
        line_heights[i] = 0.0;
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_CODE_START || sl->type == LINE_CODE_END) continue;

        char markup[MAX_LINE_LEN * 8];
        highlighter_highlight(sl->text, lang, s->theme, markup, sizeof(markup));
        pango_layout_set_markup(lay_code, markup, -1);

        int tw, th;
        pango_layout_get_pixel_size(lay_code, &tw, &th);
        
        double min_h = 24.0 * s->font_scale;
        double actual_h = (double)th;
        if (actual_h < min_h) {
            actual_h = min_h;
        }
        line_heights[i] = actual_h;
        lines_h_sum += actual_h;
    }

    double total_h = lines_h_sum + 20.0 * s->font_scale;
    if (total_h < 20.0 * s->font_scale) total_h = 20.0 * s->font_scale;

    // Fondo del bloque
    set_color(cr, s->theme->code_bg_r, s->theme->code_bg_g, s->theme->code_bg_b);
    cairo_rectangle(cr, x, y, max_w, total_h);
    cairo_fill(cr);

    // Color de texto por defecto para lo que no tenga span
    set_color(cr, s->theme->code_txt_r, s->theme->code_txt_g, s->theme->code_txt_b);

    double cur_y = y + 10.0 * s->font_scale;
    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_CODE_START || sl->type == LINE_CODE_END) continue;

        char markup[MAX_LINE_LEN * 8];
        highlighter_highlight(sl->text, lang, s->theme, markup, sizeof(markup));
        pango_layout_set_markup(lay_code, markup, -1);
        
        cairo_move_to(cr, x + 15.0 * s->font_scale, cur_y);
        pango_cairo_show_layout(cr, lay_code);
        cur_y += line_heights[i];
    }

    free(line_heights);
    return total_h;
}

static void do_transition(Slider *s, int from_idx, int to_idx, cairo_t *cr,
                          int win_w, int win_h, double progress) {
    if (from_idx == to_idx || from_idx < 0 || from_idx >= s->n_slides) {
        slider_render(s, to_idx, cr, win_w, win_h, 0.0);
        return;
    }

    cairo_surface_t *target = cairo_get_target(cr);
    cairo_surface_t *sfc_from = cairo_surface_create_similar(target,
        CAIRO_CONTENT_COLOR, win_w, win_h);
    cairo_surface_t *sfc_to = cairo_surface_create_similar(target,
        CAIRO_CONTENT_COLOR, win_w, win_h);

    if (!sfc_from || !sfc_to) {
        if (sfc_from) cairo_surface_destroy(sfc_from);
        if (sfc_to) cairo_surface_destroy(sfc_to);
        slider_render(s, to_idx, cr, win_w, win_h, 0.0);
        return;
    }

    cairo_t *cr_from = cairo_create(sfc_from);
    cairo_t *cr_to = cairo_create(sfc_to);

    cairo_set_source_rgb(cr_from, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
    cairo_paint(cr_from);
    cairo_set_source_rgb(cr_to, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
    cairo_paint(cr_to);

    // Save and clear ALL transition state to prevent recursive rendering
    TransitionType saved_tt = s->transition_type;
    int saved_from = s->transition_from;
    TransitionType saved_from_trans = s->slides[from_idx].transition;
    TransitionType saved_to_trans = s->slides[to_idx].transition;
    s->transition_type = TRANS_NONE;
    s->slides[from_idx].transition = TRANS_NONE;
    s->slides[to_idx].transition = TRANS_NONE;

    slider_render(s, from_idx, cr_from, win_w, win_h, 0.0);
    slider_render(s, to_idx, cr_to, win_w, win_h, 0.0);

    s->transition_type = saved_tt;
    s->transition_from = saved_from;
    s->slides[from_idx].transition = saved_from_trans;
    s->slides[to_idx].transition = saved_to_trans;

    TransitionType tt = saved_tt;
    if (tt == TRANS_NONE) tt = s->slides[to_idx].transition;

    double p = progress;
    if (p < 0.0) p = 0.0;
    if (p > 1.0) p = 1.0;

    switch (tt) {
    case TRANS_FADE: {
        cairo_set_source_surface(cr, sfc_from, 0, 0);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, 0, 0);
        cairo_paint_with_alpha(cr, p);
        break;
    }
    case TRANS_SLIDE_LEFT: {
        double ox = (1.0 - p) * win_w;
        cairo_set_source_surface(cr, sfc_from, -win_w + ox, 0);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, ox, 0);
        cairo_paint(cr);
        break;
    }
    case TRANS_SLIDE_RIGHT: {
        double ox = -(1.0 - p) * win_w;
        cairo_set_source_surface(cr, sfc_from, win_w + ox, 0);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, ox, 0);
        cairo_paint(cr);
        break;
    }
    case TRANS_SLIDE_UP: {
        double oy = (1.0 - p) * win_h;
        cairo_set_source_surface(cr, sfc_from, 0, -win_h + oy);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, 0, oy);
        cairo_paint(cr);
        break;
    }
    case TRANS_SLIDE_DOWN: {
        double oy = -(1.0 - p) * win_h;
        cairo_set_source_surface(cr, sfc_from, 0, win_h + oy);
        cairo_paint(cr);
        cairo_set_source_surface(cr, sfc_to, 0, oy);
        cairo_paint(cr);
        break;
    }
    default:
        cairo_set_source_surface(cr, sfc_to, 0, 0);
        cairo_paint(cr);
        break;
    }

    cairo_destroy(cr_to);
    cairo_destroy(cr_from);
    cairo_surface_destroy(sfc_to);
    cairo_surface_destroy(sfc_from);
}

void slider_render(Slider *s, int index, cairo_t *cr, int win_w, int win_h, double time_ms) {
    if (!s || index < 0 || index >= s->n_slides) return;

    TransitionType trans_type = s->transition_type;
    if (trans_type == TRANS_NONE) trans_type = s->slides[index].transition;
    double trans_ms = TRANSITION_DEFAULT_MS;

    if (trans_type != TRANS_NONE && time_ms < trans_ms && s->transition_from != index) {
        double progress = time_ms / trans_ms;
        progress = progress * progress * (3.0 - 2.0 * progress);
        do_transition(s, s->transition_from, index, cr, win_w, win_h, progress);
        return;
    }

    // Clear transition state once complete
    if (s->transition_type != TRANS_NONE && time_ms >= trans_ms) {
        s->transition_type = TRANS_NONE;
    }

    Slide *slide = &s->slides[index];
    double content_w = win_w - MARGIN_X * 2.0;
    
    slide->has_anim = false;

    char f_title[128], f_subtitle[128], f_body[128], f_bullet[128], f_num[128], f_code[128];
    snprintf(f_title,    sizeof(f_title),    "%s Bold %d", s->font_family, (int)(44 * s->font_scale));
    snprintf(f_subtitle, sizeof(f_subtitle), "%s %d",      s->font_family, (int)(26 * s->font_scale));
    snprintf(f_body,     sizeof(f_body),     "%s %d",      s->font_family, (int)(20 * s->font_scale));
    snprintf(f_bullet,   sizeof(f_bullet),   "%s %d",      s->font_family, (int)(18 * s->font_scale));
    snprintf(f_num,      sizeof(f_num),      "%s %d",      s->font_family, (int)(13 * s->font_scale));
    snprintf(f_code,     sizeof(f_code),     "Monospace %d",             (int)(16 * s->font_scale));

    PangoLayout *lay_title    = make_layout(cr, f_title,    content_w);
    PangoLayout *lay_subtitle = make_layout(cr, f_subtitle, content_w);
    PangoLayout *lay_body     = make_layout(cr, f_body,     content_w);
    PangoLayout *lay_bullet   = make_layout(cr, f_body,     content_w - 30.0 * s->font_scale);
    PangoLayout *lay_bullet2  = make_layout(cr, f_bullet,   content_w - 60.0 * s->font_scale);
    PangoLayout *lay_num      = make_layout(cr, f_num,      200.0 * s->font_scale);
    PangoLayout *lay_code     = make_layout(cr, f_code,     content_w - 30.0 * s->font_scale);

    double y = MARGIN_Y;
    int i = 0;
    while (i < slide->nlines) {
        const SlideLine *sl = &slide->lines[i];
        switch (sl->type) {
        case LINE_EMPTY: y += 12.0 * s->font_scale; i++; break;
        case LINE_TITLE:
            set_color(cr, s->theme->title_r, s->theme->title_g, s->theme->title_b);
            y += render_pango(cr, lay_title, sl->text, MARGIN_X, y);
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_set_line_width(cr, 2.5 * s->font_scale);
            cairo_move_to(cr, MARGIN_X, y + 8.0 * s->font_scale);
            cairo_line_to(cr, MARGIN_X + content_w, y + 8.0 * s->font_scale);
            cairo_stroke(cr);
            y += 22.0 * s->font_scale; i++; break;
        case LINE_SUBTITLE:
            set_color(cr, s->theme->sub_r, s->theme->sub_g, s->theme->sub_b);
            y += render_pango(cr, lay_subtitle, sl->text, MARGIN_X, y);
            y += 14.0 * s->font_scale; i++; break;
        case LINE_BODY:
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            y += render_pango(cr, lay_body, sl->text, MARGIN_X, y);
            y += 8.0 * s->font_scale; i++; break;
        case LINE_BLOCKQUOTE: {
            double b_x = MARGIN_X + 10.0 * s->font_scale;
            double t_x = b_x + 25.0 * s->font_scale;
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_set_line_width(cr, 4.0 * s->font_scale);
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            cairo_move_to(cr, b_x, y);
            cairo_line_to(cr, b_x, y + (double)th);
            cairo_stroke(cr);
            set_color(cr, s->theme->sub_r, s->theme->sub_g, s->theme->sub_b);
            cairo_move_to(cr, t_x, y);
            pango_cairo_show_layout(cr, lay_body);
            y += (double)th + 12.0 * s->font_scale;
            i++; break;
        }
        case LINE_BULLET1: {
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_bullet, markup, -1);
            
            PangoLayoutIter *iter = pango_layout_get_iter(lay_bullet);
            PangoRectangle logical_rect;
            pango_layout_iter_get_line_extents(iter, NULL, &logical_rect);
            pango_layout_iter_free(iter);
            double line_h = (double)logical_rect.height / PANGO_SCALE;
            double bullet_y = y + line_h / 2.0;

            set_color(cr, s->theme->bullet_r, s->theme->bullet_g, s->theme->bullet_b);
            cairo_arc(cr, MARGIN_X + 8.0 * s->font_scale, bullet_y, 4.0 * s->font_scale, 0, 2 * M_PI);
            cairo_fill(cr);
            
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            int tw, th;
            pango_layout_get_pixel_size(lay_bullet, &tw, &th);
            cairo_move_to(cr, MARGIN_X + 22.0 * s->font_scale, y);
            pango_cairo_show_layout(cr, lay_bullet);
            y += (double)th + 6.0 * s->font_scale;
            i++; break;
        }
        case LINE_NUM_LIST:
            set_color(cr, s->theme->bullet_r, s->theme->bullet_g, s->theme->bullet_b);
            render_pango(cr, lay_bullet, sl->marker, MARGIN_X + 2.0 * s->font_scale, y);
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            y += render_pango(cr, lay_bullet, sl->text, MARGIN_X + 40.0 * s->font_scale, y) + 6.0 * s->font_scale;
            i++; break;
        case LINE_TASK_UNCHECKED:
        case LINE_TASK_CHECKED: {
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_bullet, markup, -1);
            
            PangoLayoutIter *iter = pango_layout_get_iter(lay_bullet);
            PangoRectangle logical_rect;
            pango_layout_iter_get_line_extents(iter, NULL, &logical_rect);
            pango_layout_iter_free(iter);
            double line_h = (double)logical_rect.height / PANGO_SCALE;
            double center_y = y + line_h / 2.0;

            double sz = 18.0 * s->font_scale;
            double bx = MARGIN_X + 2.0 * s->font_scale;
            double by = center_y - sz / 2.0;

            // Dibujar caja
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_set_line_width(cr, 1.5 * s->font_scale);
            cairo_rectangle(cr, bx, by, sz, sz);
            cairo_stroke(cr);
            if (sl->type == LINE_TASK_CHECKED) {
                // Dibujar Checkmark
                cairo_move_to(cr, bx + 4.0 * s->font_scale, by + 9.0 * s->font_scale);
                cairo_line_to(cr, bx + 8.0 * s->font_scale, by + 13.0 * s->font_scale);
                cairo_line_to(cr, bx + 14.0 * s->font_scale, by + 5.0 * s->font_scale);
                cairo_stroke(cr);
            }
            
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            int tw, th;
            pango_layout_get_pixel_size(lay_bullet, &tw, &th);
            cairo_move_to(cr, MARGIN_X + 30.0 * s->font_scale, y);
            pango_cairo_show_layout(cr, lay_bullet);
            y += (double)th + 6.0 * s->font_scale;
            i++; break;
        }
        case LINE_BULLET2: {
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_bullet2, markup, -1);
            
            PangoLayoutIter *iter = pango_layout_get_iter(lay_bullet2);
            PangoRectangle logical_rect;
            pango_layout_iter_get_line_extents(iter, NULL, &logical_rect);
            pango_layout_iter_free(iter);
            double line_h = (double)logical_rect.height / PANGO_SCALE;
            double bullet_y = y + line_h / 2.0;

            double bx = MARGIN_X + 38.0 * s->font_scale;
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_move_to(cr, bx, bullet_y - 4.0 * s->font_scale); 
            cairo_line_to(cr, bx + 4.0 * s->font_scale, bullet_y);
            cairo_line_to(cr, bx, bullet_y + 4.0 * s->font_scale); 
            cairo_line_to(cr, bx - 4.0 * s->font_scale, bullet_y);
            cairo_close_path(cr); cairo_fill(cr);
            
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            int tw, th;
            pango_layout_get_pixel_size(lay_bullet2, &tw, &th);
            cairo_move_to(cr, MARGIN_X + 52.0 * s->font_scale, y);
            pango_cairo_show_layout(cr, lay_bullet2);
            y += (double)th + 4.0 * s->font_scale;
            i++; break;
        }
        case LINE_IMAGE: {
            ImgCache *imgc = get_image_cache(sl->text);
            if (imgc && imgc->surfaces) {
                if (imgc->n_frames > 1) slide->has_anim = true;
                
                cairo_surface_t *img_sfc = NULL;
                if (imgc->n_frames == 1) {
                    img_sfc = imgc->surfaces[0];
                } else if (imgc->n_frames > 1 && imgc->total_duration > 0) {
                    int t = (int)time_ms % imgc->total_duration;
                    int acc = 0;
                    for (int k = 0; k < imgc->n_frames; k++) {
                        acc += imgc->delays[k];
                        if (acc > t) {
                            img_sfc = imgc->surfaces[k];
                            break;
                        }
                    }
                    if (!img_sfc) img_sfc = imgc->surfaces[0];
                }

                if (img_sfc) {
                    int iw = cairo_image_surface_get_width(img_sfc);
                    int ih = cairo_image_surface_get_height(img_sfc);
                    double avail_h = win_h - y - MARGIN_Y - 40.0 * s->font_scale, scale = 1.0;
                    if (iw > content_w) scale = content_w / iw;
                    if (ih * scale > avail_h) scale = avail_h / ih;
                    double dw = iw * scale, dh = ih * scale;
                    cairo_save(cr); cairo_translate(cr, MARGIN_X + (content_w - dw) / 2.0, y);
                    cairo_scale(cr, scale, scale); cairo_set_source_surface(cr, img_sfc, 0, 0);
                    cairo_paint(cr); cairo_restore(cr);
                    y += dh + 14.0 * s->font_scale;
                }
            } else {
                set_color(cr, 0.15, 0.18, 0.38); cairo_rectangle(cr, MARGIN_X, y, content_w, 80.0 * s->font_scale); cairo_fill(cr);
                set_color(cr, COL_LABEL_R, COL_LABEL_G, COL_LABEL_B);
                char msg[MAX_LINE_LEN + 32]; snprintf(msg, sizeof(msg), "⚠ No se encontró: %s", sl->text);
                render_pango(cr, lay_body, msg, MARGIN_X + 10.0 * s->font_scale, y + 28.0 * s->font_scale);
                y += 94.0 * s->font_scale;
            }
            i++; break;
        }
        case LINE_TABLE_ROW:
        case LINE_TABLE_SEP: {
            int j = i;
            while (j < slide->nlines && (slide->lines[j].type == LINE_TABLE_ROW || slide->lines[j].type == LINE_TABLE_SEP)) j++;
            y += render_table(cr, lay_body, s, (SlideLine *)slide->lines, i, j - i, MARGIN_X, y, content_w) + 14.0 * s->font_scale;
            i = j; break;
        }
        case LINE_CODE_START: {
            int j = i;
            while (j < slide->nlines && slide->lines[j].type != LINE_CODE_END) j++;
            if (j < slide->nlines) j++; // Incluir LINE_CODE_END
            y += render_code_block(cr, lay_code, s, (SlideLine *)slide->lines, i, j - i, MARGIN_X, y, content_w) + 14.0 * s->font_scale;
            i = j; break;
        }
        default: i++; break;
        }
    }
    char num_buf[32]; snprintf(num_buf, sizeof(num_buf), "%d / %d", index + 1, s->n_slides);
    set_color(cr, s->theme->num_r, s->theme->num_g, s->theme->num_b);
    pango_layout_set_width(lay_num, (int)(200.0 * s->font_scale * PANGO_SCALE)); pango_layout_set_alignment(lay_num, PANGO_ALIGN_RIGHT);
    render_pango(cr, lay_num, num_buf, win_w - MARGIN_X - 200.0 * s->font_scale, win_h - 32.0 * s->font_scale);
    double prog = (s->n_slides > 1) ? (double)index / (s->n_slides - 1) : 1.0;
    set_color(cr, s->theme->bg_r * 1.5, s->theme->bg_g * 1.5, s->theme->bg_b * 1.5); cairo_rectangle(cr, 0, win_h - 4, win_w, 4); cairo_fill(cr);
    set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b); cairo_rectangle(cr, 0, win_h - 4, win_w * prog, 4); cairo_fill(cr);
    g_object_unref(lay_title); g_object_unref(lay_subtitle); g_object_unref(lay_body);
    g_object_unref(lay_bullet); g_object_unref(lay_bullet2); g_object_unref(lay_num);
    g_object_unref(lay_code);
}

int slider_export_png(Slider *s, int index, const char *path, int w, int h) {
    if (!s || index < 0 || index >= s->n_slides) return -1;
    
    cairo_surface_t *sfc = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
    cairo_t *cr = cairo_create(sfc);
    
    // Pintar fondo
    set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
    cairo_paint(cr);
    
    // Exportación estática: deshabilitar transición para evitar renderizar slide incorrecto
    TransitionType saved = s->slides[index].transition;
    s->slides[index].transition = TRANS_NONE;
    slider_render(s, index, cr, w, h, 0.0);
    s->slides[index].transition = saved;
    
    cairo_status_t status = cairo_surface_write_to_png(sfc, path);
    
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);
    
    return (status == CAIRO_STATUS_SUCCESS) ? 0 : -1;
}

int slider_export_pdf(Slider *s, const char *path, int w, int h) {
    if (!s || s->n_slides <= 0) return -1;

    cairo_surface_t *sfc = cairo_pdf_surface_create(path, (double)w, (double)h);
    if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS) {
        return -1;
    }

    cairo_t *cr = cairo_create(sfc);

    for (int i = 0; i < s->n_slides; i++) {
        // Fondo
        set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
        cairo_paint(cr);

        // Exportación estática: deshabilitar transición para evitar renderizar slide incorrecto
        TransitionType saved = s->slides[i].transition;
        s->slides[i].transition = TRANS_NONE;
        slider_render(s, i, cr, w, h, 0.0);
        s->slides[i].transition = saved;

        // Nueva página si no es la última
        cairo_show_page(cr);
    }

    cairo_status_t status = cairo_surface_status(sfc);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return (status == CAIRO_STATUS_SUCCESS) ? 0 : -1;
}

// ── Minimal GIF Writer ────────────────────────────────────────────────────────
// Implements GIF89a with LZW compression for animated slides export

#define GIF_PAL_SIZE 256

typedef struct {
    FILE *fp;
    int w, h;
    int n_frames;
} GifWriter;

static int gif_palette[GIF_PAL_SIZE];

static void gif_build_palette(void) {
    static int built = 0;
    if (built) return;
    built = 1;
    int i = 0;
    for (int r = 0; r < 6; r++)
        for (int g = 0; g < 6; g++)
            for (int b = 0; b < 6; b++)
                gif_palette[i++] = (r * 51) << 16 | (g * 51) << 8 | (b * 51);
    for (int gr = 0; gr < 40; gr++)
        gif_palette[i++] = (gr * 255 / 39) << 16 | (gr * 255 / 39) << 8 | (gr * 255 / 39);
}

static int gif_color_index(int r, int g, int b) {
    int best = 0, best_dist = 256 * 256 * 3 + 1;
    for (int i = 0; i < GIF_PAL_SIZE; i++) {
        int pr = (gif_palette[i] >> 16) & 0xFF;
        int pg = (gif_palette[i] >> 8) & 0xFF;
        int pb = gif_palette[i] & 0xFF;
        int dr = r - pr, dg = g - pg, db = b - pb;
        int dist = dr * dr + dg * dg + db * db;
        if (dist < best_dist) { best_dist = dist; best = i; }
    }
    return best;
}

static void gif_write(FILE *fp, const void *data, int len) {
    fwrite(data, 1, len, fp);
}

static void gif_putc(FILE *fp, unsigned char c) {
    fputc(c, fp);
}

static void gif_write_16(FILE *fp, unsigned short v) {
    gif_putc(fp, v & 0xFF);
    gif_putc(fp, (v >> 8) & 0xFF);
}

static void gif_write_palette(FILE *fp) {
    for (int i = 0; i < GIF_PAL_SIZE; i++) {
        gif_putc(fp, (gif_palette[i] >> 16) & 0xFF);
        gif_putc(fp, (gif_palette[i] >> 8) & 0xFF);
        gif_putc(fp, gif_palette[i] & 0xFF);
    }
}

static void gif_write_header(FILE *fp, int w, int h) {
    unsigned char header[] = "GIF89a";
    gif_write(fp, header, 6);
    gif_write_16(fp, w);
    gif_write_16(fp, h);
    // Packed field: GCT flag=1, color res=7, sort=0, GCT size=7 (2^(7+1)=256)
    gif_putc(fp, 0x80 | 0x70 | 7);
    gif_putc(fp, 0);    // bg color index
    gif_putc(fp, 0);    // pixel aspect ratio
    gif_write_palette(fp);
}

static void gif_write_gce(FILE *fp, int delay_cs) {
    gif_putc(fp, 0x21);            // extension introducer
    gif_putc(fp, 0xF9);            // graphic control label
    gif_putc(fp, 0x04);            // block size
    gif_putc(fp, 0x00);            // packed: no transparency, no user input, disposal=0
    gif_write_16(fp, delay_cs);    // delay in centiseconds
    gif_putc(fp, 0x00);            // transparent color index
    gif_putc(fp, 0x00);            // block terminator
}

static int lzw_find(int *key, int *val, int *pref, int *suff, int p, int s) {
    int h = (p * 256 + s) % 5003;
    int start = h;
    do {
        if (key[h] == -1) return -1;
        int code = val[h];
        if (code >= 256 && pref[code] == p && suff[code] == s) return code;
        h = (h + 1) % 5003;
    } while (h != start);
    return -1;
}

static void gif_write_image(FILE *fp, const unsigned char *indexed, int w, int h) {
    gif_putc(fp, 0x2C);
    gif_write_16(fp, 0);
    gif_write_16(fp, 0);
    gif_write_16(fp, w);
    gif_write_16(fp, h);
    gif_putc(fp, 0x00);

    int min_code_size = 8;
    gif_putc(fp, min_code_size);

    int *key = calloc(5003, sizeof(int));
    int *val = calloc(5003, sizeof(int));
    int *pref = calloc(4096, sizeof(int));
    int *suff = calloc(4096, sizeof(int));

    if (!key || !val || !pref || !suff) {
        free(key); free(val); free(pref); free(suff);
        return;
    }

    memset(key, -1, 5003 * sizeof(int));
    memset(pref, -1, 4096 * sizeof(int));

    for (int i = 0; i < 256; i++) {
        int h = i % 5003;
        while (key[h] != -1) h = (h + 1) % 5003;
        key[h] = i; val[h] = i;
        pref[i] = -1; suff[i] = i;
    }

    int next = 258, csize = 9, cmax = 512, clr = 256, eoi = 257;

    unsigned char buf[256];
    int blen = 0, bbits = 0, bcnt = 0;

    #define LZWC(c) do { \
        bbits |= (c) << bcnt; bcnt += csize; \
        while (bcnt >= 8) { \
            buf[blen++] = bbits & 0xFF; bbits >>= 8; bcnt -= 8; \
            if (blen >= 255) { gif_putc(fp, blen); gif_write(fp, buf, blen); blen = 0; } \
        } \
    } while (0)

    LZWC(clr);
    int cur = indexed[0];
    int n = w * h;

    for (int pos = 1; pos < n; pos++) {
        int byte = indexed[pos];
        int found = lzw_find(key, val, pref, suff, cur, byte);

        if (found >= 0) {
            cur = found;
            continue;
        }

        LZWC(cur);

        if (next < 4096) {
            int h = (cur * 256 + byte) % 5003;
            while (key[h] != -1) h = (h + 1) % 5003;
            key[h] = cur * 256 + byte; val[h] = next;
            pref[next] = cur; suff[next] = byte;
            next++;
            if (next > cmax) { csize++; cmax = 1 << csize; }
        }

        cur = byte;

        if (next >= 4096) {
            LZWC(clr);
            memset(key, -1, 5003 * sizeof(int));
            memset(pref, -1, 4096 * sizeof(int));
            for (int i = 0; i < 256; i++) {
                int hi = i % 5003;
                while (key[hi] != -1) hi = (hi + 1) % 5003;
                key[hi] = i; val[hi] = i;
                pref[i] = -1; suff[i] = i;
            }
            next = 258; csize = 9; cmax = 512;
        }
    }

    LZWC(cur);
    LZWC(eoi);
    if (bcnt) { buf[blen++] = bbits & 0xFF; bbits = 0; bcnt = 0; }
    if (blen) { gif_putc(fp, blen); gif_write(fp, buf, blen); }
    gif_putc(fp, 0x00);

    free(key); free(val); free(pref); free(suff);
}

static int gif_open(GifWriter *gw, const char *path, int w, int h) {
    gif_build_palette();
    gw->fp = fopen(path, "wb");
    if (!gw->fp) return 0;
    gw->w = w;
    gw->h = h;
    gw->n_frames = 0;
    gif_write_header(gw->fp, w, h);
    return 1;
}

static int gif_add_frame(GifWriter *gw, const unsigned char *rgba, int delay_cs) {
    if (!gw->fp) return 0;
    int w = gw->w, h = gw->h;

    // Quantize RGBA to indexed
    unsigned char *indexed = malloc(w * h);
    if (!indexed) return 0;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int off = (y * w + x) * 4;
            indexed[y * w + x] = gif_color_index(rgba[off + 2], rgba[off + 1], rgba[off]);
        }
    }

    gif_write_gce(gw->fp, delay_cs);
    gif_write_image(gw->fp, indexed, w, h);
    free(indexed);
    gw->n_frames++;
    return 1;
}

static void gif_close(GifWriter *gw) {
    if (gw->fp) {
        gif_putc(gw->fp, 0x3B); // trailer
        fclose(gw->fp);
        gw->fp = NULL;
    }
}

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
            // Cairo uses premultiplied ARGB32 (host byte order, typically BGRA on little-endian)
            // Convert to RGBA
            rgba_out[off + 0] = (argb >> 16) & 0xFF; // R
            rgba_out[off + 1] = (argb >> 8) & 0xFF;  // G
            rgba_out[off + 2] = argb & 0xFF;          // B
            rgba_out[off + 3] = (argb >> 24) & 0xFF;  // A
        }
    }
}

// ── GIF Export ─────────────────────────────────────────────────────────────────

int slider_export_gif(Slider *s, const char *path, int w, int h) {
    if (!s || s->n_slides <= 0) return -1;

    // Parámetros de animación
    int fps = 15;
    int hold_frames = fps;           // 1 segundo por slide estático
    int trans_ms = TRANSITION_DEFAULT_MS; // 300ms
    int trans_frames = (int)(trans_ms / 1000.0 * fps + 0.5);
    if (trans_frames < 1) trans_frames = 1;
    int delay_cs = 100 / fps;        // delay por frame en centisegundos

    // Crear surface temporal para renderizar frames
    cairo_surface_t *sfc = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
    cairo_t *cr = cairo_create(sfc);

    // Buffer para datos RGBA de cada frame
    unsigned char *frame_rgba = malloc(w * h * 4);
    if (!frame_rgba) { cairo_destroy(cr); cairo_surface_destroy(sfc); return -1; }

    GifWriter gw;
    if (!gif_open(&gw, path, w, h)) {
        free(frame_rgba);
        cairo_destroy(cr);
        cairo_surface_destroy(sfc);
        return -1;
    }

    for (int i = 0; i < s->n_slides; i++) {
        if (i == 0) {
            // Primer slide: solo hold frames (sin transición de entrada)
            for (int f = 0; f < hold_frames; f++) {
                set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
                cairo_paint(cr);
                TransitionType saved = s->slides[i].transition;
                s->slides[i].transition = TRANS_NONE;
                slider_render(s, i, cr, w, h, (double)trans_ms + 1.0);
                s->slides[i].transition = saved;
                cairo_surface_flush(sfc);
                gif_frame_from_surface(sfc, frame_rgba);
                gif_add_frame(&gw, frame_rgba, delay_cs);
            }
        } else {
            // Transición desde slide anterior
            TransitionType trans = s->slides[i].transition;
            if (trans == TRANS_NONE) {
                // Sin transición: solo hold frames
                for (int f = 0; f < hold_frames; f++) {
                    set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
                    cairo_paint(cr);
                    TransitionType saved = s->slides[i].transition;
                    s->slides[i].transition = TRANS_NONE;
                    slider_render(s, i, cr, w, h, (double)trans_ms + 1.0);
                    s->slides[i].transition = saved;
                    cairo_surface_flush(sfc);
                    gif_frame_from_surface(sfc, frame_rgba);
                    gif_add_frame(&gw, frame_rgba, delay_cs);
                }
            } else {
                // Renderizar frames de transición
                for (int f = 0; f < trans_frames; f++) {
                    double t = (double)f / trans_frames * trans_ms;
                    set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
                    cairo_paint(cr);
                    s->transition_type = trans;
                    s->transition_from = i - 1;
                    slider_render(s, i, cr, w, h, t);
                    cairo_surface_flush(sfc);
                    gif_frame_from_surface(sfc, frame_rgba);
                    gif_add_frame(&gw, frame_rgba, delay_cs);
                }
                // Hold frames después de la transición
                for (int f = 0; f < hold_frames; f++) {
                    set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
                    cairo_paint(cr);
                    TransitionType saved = s->slides[i].transition;
                    s->slides[i].transition = TRANS_NONE;
                    slider_render(s, i, cr, w, h, (double)trans_ms + 1.0);
                    s->slides[i].transition = saved;
                    cairo_surface_flush(sfc);
                    gif_frame_from_surface(sfc, frame_rgba);
                    gif_add_frame(&gw, frame_rgba, delay_cs);
                }
            }
        }
    }

    gif_close(&gw);
    free(frame_rgba);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return (gw.n_frames > 0) ? 0 : -1;
}

int slider_export_svg(Slider *s, int index, const char *path, int w, int h) {
    if (!s || index < 0 || index >= s->n_slides) return -1;

    cairo_surface_t *sfc = cairo_svg_surface_create(path, (double)w, (double)h);
    if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS) {
        return -1;
    }

    cairo_t *cr = cairo_create(sfc);

    set_color(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
    cairo_paint(cr);

    TransitionType saved = s->slides[index].transition;
    s->slides[index].transition = TRANS_NONE;
    slider_render(s, index, cr, w, h, 0.0);
    s->slides[index].transition = saved;

    cairo_status_t status = cairo_surface_status(sfc);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return (status == CAIRO_STATUS_SUCCESS) ? 0 : -1;
}
