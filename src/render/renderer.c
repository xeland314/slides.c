#include "../../slider.h"
#include "../core/internal.h"
#include "../core/highlighter.h"
#include <pango/pangocairo.h>
#include <cairo/cairo-pdf.h>
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
            double w = (double)tw + 24.0; 
            if (w > desired_w[c]) desired_w[c] = w;
        }
    }

    for (int c = 0; c < max_cols; c++) {
        if (desired_w[c] < 40.0) desired_w[c] = 40.0;
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
        double row_h = 40.0; 
        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            pango_layout_set_width(lay_body, (int)((col_w[c] - 12.0) * PANGO_SCALE));
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            if ((double)th + 16.0 > row_h) row_h = (double)th + 16.0;
        }

        // Fill background
        if (is_header) set_color(cr, s->theme->table_hdr_r, s->theme->table_hdr_g, s->theme->table_hdr_b);
        else if (data_row % 2 == 0) set_color(cr, s->theme->table_row_r, s->theme->table_row_g, s->theme->table_row_b);
        else set_color(cr, s->theme->table_alt_r, s->theme->table_alt_g, s->theme->table_alt_b);

        cairo_rectangle(cr, x, cur_y, max_w, row_h);
        cairo_fill(cr);

        // Borders
        set_color(cr, s->theme->table_bdr_r, s->theme->table_bdr_g, s->theme->table_bdr_b);
        cairo_set_line_width(cr, 0.5);
        
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
            pango_layout_set_width(lay_body, (int)((col_w[c] - 12.0) * PANGO_SCALE));
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            cairo_move_to(cr, cur_text_x + 6.0, cur_y + (row_h - (double)th) / 2.0);
            pango_cairo_show_layout(cr, lay_body);
            cur_text_x += col_w[c];
        }
        cur_y += row_h;
        if (!is_header) data_row++;
    }
    set_color(cr, s->theme->table_bdr_r, s->theme->table_bdr_g, s->theme->table_bdr_b);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, x, y, max_w, cur_y - y);
    cairo_stroke(cr);
    pango_layout_set_width(lay_body, (int)((max_w) * PANGO_SCALE));
    return cur_y - y;
}

static double render_code_block(cairo_t *cr, PangoLayout *lay_code,
                               Slider *s, SlideLine *lines, int start, int count,
                               double x, double y, double max_w) {
    if (count <= 0) return 0.0;

    // Calcular altura total primero
    double line_h = 24.0 * s->font_scale;
    double total_h = (count - 2) * line_h + 20.0; // -2 por START y END
    if (total_h < 20.0) total_h = 20.0;

    // Fondo del bloque
    set_color(cr, s->theme->code_bg_r, s->theme->code_bg_g, s->theme->code_bg_b);
    cairo_rectangle(cr, x, y, max_w, total_h);
    cairo_fill(cr);

    // Color de texto por defecto para lo que no tenga span
    set_color(cr, s->theme->code_txt_r, s->theme->code_txt_g, s->theme->code_txt_b);

    double cur_y = y + 10.0;
    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_CODE_START || sl->type == LINE_CODE_END) continue;

        char markup[MAX_LINE_LEN * 8];
        highlighter_highlight(sl->text, s->theme, markup, sizeof(markup));
        pango_layout_set_markup(lay_code, markup, -1);
        
        cairo_move_to(cr, x + 15.0, cur_y);
        pango_cairo_show_layout(cr, lay_code);
        cur_y += line_h;
    }

    return total_h;
}

void slider_render(Slider *s, int index, cairo_t *cr, int win_w, int win_h, double time_ms) {
    if (!s || index < 0 || index >= s->n_slides) return;
    Slide *slide = &s->slides[index];
    double content_w = win_w - MARGIN_X * 2.0;
    
    // Resetear flag de animación en cada render para recalcularlo si es necesario
    // O mejor: lo asumimos falso y si encontramos un gif lo ponemos a true.
    // Esto es importante para el backend.
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
    PangoLayout *lay_bullet   = make_layout(cr, f_body,     content_w - 30);
    PangoLayout *lay_bullet2  = make_layout(cr, f_bullet,   content_w - 60);
    PangoLayout *lay_num      = make_layout(cr, f_num,      200);
    PangoLayout *lay_code     = make_layout(cr, f_code,     content_w - 30);

    double y = MARGIN_Y;
    int i = 0;
    while (i < slide->nlines) {
        const SlideLine *sl = &slide->lines[i];
        switch (sl->type) {
        case LINE_EMPTY: y += 12.0; i++; break;
        case LINE_TITLE:
            set_color(cr, s->theme->title_r, s->theme->title_g, s->theme->title_b);
            y += render_pango(cr, lay_title, sl->text, MARGIN_X, y);
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_set_line_width(cr, 2.5);
            cairo_move_to(cr, MARGIN_X, y + 8);
            cairo_line_to(cr, MARGIN_X + content_w, y + 8);
            cairo_stroke(cr);
            y += 22.0; i++; break;
        case LINE_SUBTITLE:
            set_color(cr, s->theme->sub_r, s->theme->sub_g, s->theme->sub_b);
            y += render_pango(cr, lay_subtitle, sl->text, MARGIN_X, y);
            y += 14.0; i++; break;
        case LINE_BODY:
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            y += render_pango(cr, lay_body, sl->text, MARGIN_X, y);
            y += 8.0; i++; break;
        case LINE_BLOCKQUOTE: {
            double b_x = MARGIN_X + 10.0;
            double t_x = b_x + 25.0;
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_set_line_width(cr, 4.0);
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
            y += (double)th + 12.0;
            i++; break;
        }
        case LINE_BULLET1:
            set_color(cr, s->theme->bullet_r, s->theme->bullet_g, s->theme->bullet_b);
            cairo_arc(cr, MARGIN_X + 8, y + 11, 4, 0, 2 * M_PI);
            cairo_fill(cr);
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            y += render_pango(cr, lay_bullet, sl->text, MARGIN_X + 22.0, y) + 6.0;
            i++; break;
        case LINE_TASK_UNCHECKED:
        case LINE_TASK_CHECKED: {
            double sz = 18.0;
            double bx = MARGIN_X + 2, by = y + 2;
            // Dibujar caja
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_set_line_width(cr, 1.5);
            cairo_rectangle(cr, bx, by, sz, sz);
            cairo_stroke(cr);
            if (sl->type == LINE_TASK_CHECKED) {
                // Dibujar Checkmark
                cairo_move_to(cr, bx + 4, by + 9);
                cairo_line_to(cr, bx + 8, by + 13);
                cairo_line_to(cr, bx + 14, by + 5);
                cairo_stroke(cr);
            }
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            y += render_pango(cr, lay_bullet, sl->text, MARGIN_X + 30.0, y) + 6.0;
            i++; break;
        }
        case LINE_BULLET2: {
            double bx = MARGIN_X + 38, by = y + 10;
            set_color(cr, s->theme->accent_r, s->theme->accent_g, s->theme->accent_b);
            cairo_move_to(cr, bx, by - 4); cairo_line_to(cr, bx + 4, by);
            cairo_line_to(cr, bx, by + 4); cairo_line_to(cr, bx - 4, by);
            cairo_close_path(cr); cairo_fill(cr);
            set_color(cr, s->theme->body_r, s->theme->body_g, s->theme->body_b);
            y += render_pango(cr, lay_bullet2, sl->text, MARGIN_X + 52.0, y) + 4.0;
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
                    double avail_h = win_h - y - MARGIN_Y - 40.0, scale = 1.0;
                    if (iw > content_w) scale = content_w / iw;
                    if (ih * scale > avail_h) scale = avail_h / ih;
                    double dw = iw * scale, dh = ih * scale;
                    cairo_save(cr); cairo_translate(cr, MARGIN_X + (content_w - dw) / 2.0, y);
                    cairo_scale(cr, scale, scale); cairo_set_source_surface(cr, img_sfc, 0, 0);
                    cairo_paint(cr); cairo_restore(cr);
                    y += dh + 14.0;
                }
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
            y += render_table(cr, lay_body, s, (SlideLine *)slide->lines, i, j - i, MARGIN_X, y, content_w) + 14.0;
            i = j; break;
        }
        case LINE_CODE_START: {
            int j = i;
            while (j < slide->nlines && slide->lines[j].type != LINE_CODE_END) j++;
            if (j < slide->nlines) j++; // Incluir LINE_CODE_END
            y += render_code_block(cr, lay_code, s, (SlideLine *)slide->lines, i, j - i, MARGIN_X, y, content_w) + 14.0;
            i = j; break;
        }
        default: i++; break;
        }
    }
    char num_buf[32]; snprintf(num_buf, sizeof(num_buf), "%d / %d", index + 1, s->n_slides);
    set_color(cr, s->theme->num_r, s->theme->num_g, s->theme->num_b);
    pango_layout_set_width(lay_num, 200 * PANGO_SCALE); pango_layout_set_alignment(lay_num, PANGO_ALIGN_RIGHT);
    render_pango(cr, lay_num, num_buf, win_w - MARGIN_X - 200, win_h - 32.0);
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
    
    // Renderizar slide (tiempo 0 para exportación estática)
    slider_render(s, index, cr, w, h, 0.0);
    
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

        // Renderizar slide (tiempo 0)
        slider_render(s, i, cr, w, h, 0.0);

        // Nueva página si no es la última
        cairo_show_page(cr);
    }

    cairo_status_t status = cairo_surface_status(sfc);
    cairo_destroy(cr);
    cairo_surface_destroy(sfc);

    return (status == CAIRO_STATUS_SUCCESS) ? 0 : -1;
}
