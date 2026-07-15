#include "../../slider.h"
#include "internal.h"
#include "../render/render_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

void parse_line(const char *raw, SlideLine *out) {
    char buf[MAX_LINE_LEN];
    strncpy(buf, raw, MAX_LINE_LEN - 1);
    buf[MAX_LINE_LEN - 1] = '\0';
    // Quitar \r\n
    char *p = buf + strlen(buf) - 1;
    while (p >= buf && (*p == '\r' || *p == '\n')) *p-- = '\0';

    out->ncols = 0;
    out->cols = NULL;
    out->img_cfg = NULL;

    // Línea vacía
    char *s = buf;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') { out->type = LINE_EMPTY; out->text[0] = '\0'; return; }

    // Títulos: ###### … # (orden descendente para evitar falsos positivos)
    if (strncmp(s, "###### ", 7) == 0) {
        out->type = LINE_H6;
        strncpy(out->text, s + 7, MAX_LINE_LEN - 1);
        return;
    }
    if (strncmp(s, "##### ", 6) == 0) {
        out->type = LINE_H5;
        strncpy(out->text, s + 6, MAX_LINE_LEN - 1);
        return;
    }
    if (strncmp(s, "#### ", 5) == 0) {
        out->type = LINE_H4;
        strncpy(out->text, s + 5, MAX_LINE_LEN - 1);
        return;
    }
    if (strncmp(s, "### ", 4) == 0) {
        out->type = LINE_H3;
        strncpy(out->text, s + 4, MAX_LINE_LEN - 1);
        return;
    }
    if (strncmp(s, "## ", 3) == 0) {
        out->type = LINE_SUBTITLE;
        strncpy(out->text, s + 3, MAX_LINE_LEN - 1);
        return;
    }
    if (strncmp(s, "# ", 2) == 0) {
        out->type = LINE_TITLE;
        strncpy(out->text, s + 2, MAX_LINE_LEN - 1);
        return;
    }
    // Blockquote
    if (strncmp(s, "> ", 2) == 0) {
        out->type = LINE_BLOCKQUOTE;
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
        char tmp[MAX_COLS][256];
        memset(tmp, 0, sizeof(tmp));
        int n = parse_table_row(s, tmp, MAX_COLS);
        if (n > 0) {
            out->cols = malloc(n * sizeof(char[256]));
            if (out->cols) memcpy(out->cols, tmp, n * sizeof(char[256]));
        }
        out->ncols = n;
        return;
    }
    // Subviñeta (dos espacios + "- ")
    if ((strncmp(s, "  - ", 4) == 0 || strncmp(buf, "  - ", 4) == 0)) {
        out->type = LINE_BULLET2;
        const char *content = strstr(buf, "- ") + 2;
        strncpy(out->text, content, MAX_LINE_LEN - 1);
        return;
    }
    // Checkbox (Task List)
    if (strncmp(s, "- [ ] ", 6) == 0) {
        out->type = LINE_TASK_UNCHECKED;
        strncpy(out->text, s + 6, MAX_LINE_LEN - 1);
        return;
    }
    if (strncmp(s, "- [x] ", 6) == 0 || strncmp(s, "- [X] ", 6) == 0) {
        out->type = LINE_TASK_CHECKED;
        strncpy(out->text, s + 6, MAX_LINE_LEN - 1);
        return;
    }

    // Listas numeradas/letras: (1. a) i. etc)
    char *dot = strpbrk(s, ".)");
    if (dot && (dot - s) < 4) { // El prefijo es corto
        // Solo si empieza con letra o número
        if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z')) {
            // Verificar que sea tipo "1. " o "a) "
            if (*(dot + 1) == ' ' || *(dot + 1) == '\t') {
                out->type = LINE_NUM_LIST;
                int marker_len = (int)(dot - s + 1);
                if (marker_len > 15) marker_len = 15;
                strncpy(out->marker, s, marker_len);
                out->marker[marker_len] = '\0';
                strncpy(out->text, trim(dot + 1), MAX_LINE_LEN - 1);
                return;
            }
        }
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

static void img_config_reset(ImageConfig *cfg) {
    cfg->opacity = 1.0;
    cfg->rotate = 0.0;
    cfg->active = 0;
    cfg->fit = IMG_FIT_NONE;
    cfg->width = -1;
    cfg->height = -1;
    cfg->width_unit = UNIT_UNSET;
    cfg->height_unit = UNIT_UNSET;
    cfg->radius = 0;
    cfg->align = IMG_ALIGN_CENTER;
}

static int parse_img_config(const char *raw, ImageConfig *cfg) {
    // raw points right after "img:" — parse key=value pairs separated by commas
    const char *p = raw;
    while (*p == ' ') p++;
    while (*p && *p != '>' && *p != '-') {
        // Skip leading whitespace
        while (*p == ' ') p++;
        if (!*p || *p == '>' || *p == '-') break;

        // Read key
        char key[32];
        int ki = 0;
        while (*p && *p != '=' && *p != ',' && *p != ' ' && *p != '>' && ki < 31)
            key[ki++] = *p++;
        key[ki] = '\0';
        // Trim trailing spaces from key
        while (ki > 0 && key[ki-1] == ' ') key[--ki] = '\0';

        if (*p != '=') { // Malformed, skip to next comma
            while (*p && *p != ',') p++;
            if (*p == ',') p++;
            continue;
        }
        p++; // skip '='

        // Read value
        char val[64];
        int vi = 0;
        // Allow '-' as first char (negative numbers), then stop at '-' for HTML comment boundaries
        if (*p == '-') val[vi++] = *p++;
        while (*p && *p != ',' && *p != '>' && *p != '-' && *p != ' ' && vi < 63)
            val[vi++] = *p++;
        val[vi] = '\0';

        // Trim trailing spaces
        while (vi > 0 && val[vi-1] == ' ') val[--vi] = '\0';

        // Apply key=value
        if (strcmp(key, "fit") == 0) {
            if (strcmp(val, "cover") == 0) cfg->fit = IMG_FIT_COVER;
            else if (strcmp(val, "contain") == 0) cfg->fit = IMG_FIT_CONTAIN;
            else if (strcmp(val, "fill") == 0) cfg->fit = IMG_FIT_FILL;
            cfg->active = 1;
        } else if (strcmp(key, "width") == 0) {
            cfg->width = atoi(val);
            cfg->width_unit = strchr(val, '%') ? UNIT_PCT : UNIT_PX;
            cfg->active = 1;
        } else if (strcmp(key, "height") == 0) {
            cfg->height = atoi(val);
            cfg->height_unit = strchr(val, '%') ? UNIT_PCT : UNIT_PX;
            cfg->active = 1;
        } else if (strcmp(key, "opacity") == 0) {
            if (strchr(val, '%')) {
                cfg->opacity = atof(val) / 100.0;
            } else {
                cfg->opacity = atof(val);
            }
            if (cfg->opacity < 0.0) cfg->opacity = 0.0;
            if (cfg->opacity > 1.0) cfg->opacity = 1.0;
            cfg->active = 1;
        } else if (strcmp(key, "radius") == 0) {
            cfg->radius = atoi(val);
            if (cfg->radius < 0) cfg->radius = 0;
            cfg->active = 1;
        } else if (strcmp(key, "rotate") == 0) {
            cfg->rotate = atof(val);
            cfg->active = 1;
        } else if (strcmp(key, "align") == 0) {
            if (strcmp(val, "left") == 0) cfg->align = IMG_ALIGN_LEFT;
            else if (strcmp(val, "right") == 0) cfg->align = IMG_ALIGN_RIGHT;
            else cfg->align = IMG_ALIGN_CENTER;
            cfg->active = 1;
        }

        // Skip commas
        while (*p == ' ' || *p == ',') p++;
    }
    return cfg->active;
}

static void theme_assign(Slider *s, const Theme *t);

Slider* slider_load(const char *path) {
    struct stat st;
    long long mtime = 0;
    if (stat(path, &st) == 0) {
        mtime = (long long)st.st_mtime;
    }
    
    FILE *fp = fopen(path, "r");
    if (!fp) { fprintf(stderr, "No se pudo abrir: %s\n", path); return NULL; }

    // Evitar cargar archivos vacíos durante escritura temporal
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size <= 0) {
        fprintf(stderr, "\033[1;33m[LINTER WARNING] Archivo vacio: %s\033[0m\n", path);
        fclose(fp);
        return NULL;
    }

    Slider *s = calloc(1, sizeof(Slider));
    if (!s) { fclose(fp); return NULL; }
    theme_assign(s, theme_default());
    strncpy(s->font_family, "Inter", sizeof(s->font_family) - 1);
    s->font_scale = 1.0;
    strncpy(s->filepath, path, sizeof(s->filepath) - 1);
    s->last_mtime = mtime;

    char line[MAX_LINE_LEN];
    char trimmed[MAX_LINE_LEN];

    // Parse Frontmatter si el archivo empieza con "---"
    if (fgets(line, sizeof(line), fp)) {
        strncpy(trimmed, line, MAX_LINE_LEN - 1);
        char *t = trim(trimmed);
        if (strcmp(t, "---") == 0) {
            while (fgets(line, sizeof(line), fp)) {
                strncpy(trimmed, line, MAX_LINE_LEN - 1);
                t = trim(trimmed);
                if (strcmp(t, "---") == 0) {
                    break;
                }
                char *colon = strchr(t, ':');
                if (colon) {
                    *colon = '\0';
                    char *key = trim(t);
                    char *val = trim(colon + 1);
                    if (strcmp(key, "theme") == 0 || strcmp(key, "palette") == 0) {
                        theme_assign(s, theme_find(val));
                    } else if (strcmp(key, "font") == 0 || strcmp(key, "font-family") == 0) {
                        strncpy(s->font_family, val, sizeof(s->font_family) - 1);
                        s->font_family[sizeof(s->font_family) - 1] = '\0';
                    } else if (strcmp(key, "scale") == 0 || strcmp(key, "font-scale") == 0) {
                        double sc = atof(val);
                        if (sc > 0.1) s->font_scale = sc;
                    } else if (strcmp(key, "colors") == 0 && val[0] == '\0') {
                        while (fgets(line, sizeof(line), fp)) {
                            strncpy(trimmed, line, MAX_LINE_LEN - 1);
                            t = trim(trimmed);
                            if (t[0] == '\0' || t[0] == '-') break;
                            char *c = strchr(t, ':');
                            if (!c) break;
                            *c = '\0';
                            char *ck = trim(t);
                            char *cv = trim(c + 1);
                            // Strip surrounding double quotes (from YAML)
                            if (cv[0] == '"') {
                                int len = (int)strlen(cv);
                                if (len > 1 && cv[len-1] == '"') cv[len-1] = '\0';
                                cv++;
                            }
                            slider_set_color(s, ck, cv);
                        }
                        if (t && t[0] == '-') {
                            if (strcmp(t, "---") == 0) break;
                            continue;
                        }
                        break;
                    }
                }
            }
        } else {
            rewind(fp);
        }
    }

    int n = 0;
    int in_code = 0;
    int in_notes = 0;
    ImageConfig pending_img_cfg;
    img_config_reset(&pending_img_cfg);

    // Set default transition for first slide
    if (n < MAX_SLIDES) {
        s->slides[n].transition = TRANS_NONE;
    }

    while (fgets(line, sizeof(line), fp) && n < MAX_SLIDES) {
        strncpy(trimmed, line, MAX_LINE_LEN - 1);
        char *t = trim(trimmed);
        
        if (!in_code && strcmp(t, "---") == 0) {
            n++;
            in_notes = 0;
            if (n < MAX_SLIDES) {
                s->slides[n].transition = TRANS_NONE;
            }
            continue;
        }

        // Notes parsing
        if (!in_code) {
            if (in_notes) {
                char *end = strstr(line, "-->");
                if (end) {
                    int len = (int)(end - line);
                    Slide *cur = &s->slides[n];
                    int cur_len = (int)strlen(cur->notes);
                    if (cur_len + len < 4090) {
                        strncat(cur->notes, line, len);
                    }
                    in_notes = 0;
                    char *notes_trimmed = trim(cur->notes);
                    if (notes_trimmed != cur->notes) {
                        memmove(cur->notes, notes_trimmed, strlen(notes_trimmed) + 1);
                    }
                } else {
                    Slide *cur = &s->slides[n];
                    int cur_len = (int)strlen(cur->notes);
                    int line_len = (int)strlen(line);
                    if (cur_len + line_len < 4090) {
                        strcat(cur->notes, line);
                    }
                }
                continue;
            } else {
                char *start = strstr(line, "<!--");
                if (start) {
                    // Check for transition directive first
                    char *trans_indicator = strstr(start, "transition:");
                    if (trans_indicator) {
                        char *val_start = trans_indicator + 11;
                        while (*val_start == ' ') val_start++;
                        char val[32];
                        int vi = 0;
                        while (*val_start && *val_start != ' ' && *val_start != '>' && vi < 31) {
                            val[vi++] = *val_start++;
                        }
                        val[vi] = '\0';
                        Slide *cur = &s->slides[n];
                        if (strcmp(val, "fade") == 0) cur->transition = TRANS_FADE;
                        else if (strcmp(val, "slide-left") == 0) cur->transition = TRANS_SLIDE_LEFT;
                        else if (strcmp(val, "slide-right") == 0) cur->transition = TRANS_SLIDE_RIGHT;
                        else if (strcmp(val, "slide-up") == 0) cur->transition = TRANS_SLIDE_UP;
                        else if (strcmp(val, "slide-down") == 0) cur->transition = TRANS_SLIDE_DOWN;
                        else if (strcmp(val, "none") == 0) cur->transition = TRANS_NONE;
                        continue;
                    }

                    char *img_indicator = strstr(start, "img:");
                    if (img_indicator) {
                        parse_img_config(img_indicator + 4, &pending_img_cfg);
                        continue;
                    }

                    char *notes_indicator = strstr(start, "notes:");
                    if (!notes_indicator) notes_indicator = strstr(start, "note:");
                    if (!notes_indicator) notes_indicator = strstr(start, "NOTES:");
                    if (!notes_indicator) notes_indicator = strstr(start, "NOTE:");
                    
                    if (notes_indicator) {
                        char *content_start = notes_indicator;
                        if (strncmp(content_start, "notes:", 6) == 0 || strncmp(content_start, "NOTES:", 6) == 0) {
                            content_start += 6;
                        } else {
                            content_start += 5;
                        }
                        char *end = strstr(content_start, "-->");
                        if (end) {
                            int len = (int)(end - content_start);
                            Slide *cur = &s->slides[n];
                            int cur_len = (int)strlen(cur->notes);
                            if (cur_len + len < 4090) {
                                strncat(cur->notes, content_start, len);
                                char *notes_trimmed = trim(cur->notes);
                                if (notes_trimmed != cur->notes) {
                                    memmove(cur->notes, notes_trimmed, strlen(notes_trimmed) + 1);
                                }
                            }
                        } else {
                            Slide *cur = &s->slides[n];
                            int len = (int)strlen(content_start);
                            if (len < 4090) {
                                strcpy(cur->notes, content_start);
                            }
                            in_notes = 1;
                        }
                        continue;
                    }
                }
            }
        }

        // Code block triple backticks
        if (strncmp(t, "```", 3) == 0) {
            Slide *cur = &s->slides[n];
            if (cur->nlines < MAX_LINES) {
                if (!in_code) {
                    cur->lines[cur->nlines].type = LINE_CODE_START;
                    strncpy(cur->lines[cur->nlines].text, t + 3, MAX_LINE_LEN - 1);
                    in_code = 1;
                } else {
                    cur->lines[cur->nlines].type = LINE_CODE_END;
                    cur->lines[cur->nlines].text[0] = '\0';
                    in_code = 0;
                }
                cur->nlines++;
            }
            continue;
        }

        Slide *cur = &s->slides[n];
        if (cur->nlines < MAX_LINES) {
            if (in_code) {
                cur->lines[cur->nlines].type = LINE_CODE;
                char *p = line + strlen(line) - 1;
                while (p >= line && (*p == '\r' || *p == '\n')) *p-- = '\0';
                strncpy(cur->lines[cur->nlines].text, line, MAX_LINE_LEN - 1);
            } else {
                parse_line(line, &cur->lines[cur->nlines]);
                // Aplicar configuración img: pendiente a la siguiente imagen
                if (pending_img_cfg.active) {
                    if (cur->lines[cur->nlines].type == LINE_IMAGE) {
                        cur->lines[cur->nlines].img_cfg = malloc(sizeof(ImageConfig));
                        if (cur->lines[cur->nlines].img_cfg)
                            *cur->lines[cur->nlines].img_cfg = pending_img_cfg;
                        img_config_reset(&pending_img_cfg);
                    } else if (cur->lines[cur->nlines].type != LINE_EMPTY) {
                        img_config_reset(&pending_img_cfg);
                    }
                }
            }
            cur->nlines++;
            if (cur->nlines >= MAX_LINES - 1) {
                fprintf(stderr, "\033[1;33m[LINTER WARNING] Slide %d en %s tiene demasiadas líneas (> %d). Algunas líneas no se mostrarán.\033[0m\n", n + 1, path, MAX_LINES);
            }
        }
    }

    if (in_code) {
        fprintf(stderr, "\033[1;31m[LINTER WARNING] Bloque de código sin cerrar al final de %s\033[0m\n", path);
    }

    s->n_slides = n + 1;
    // Trim de notas final
    for (int idx = 0; idx < s->n_slides; idx++) {
        char *t_notes = trim(s->slides[idx].notes);
        if (t_notes != s->slides[idx].notes) {
            memmove(s->slides[idx].notes, t_notes, strlen(t_notes) + 1);
        }
    }

    // Warn if the file had no meaningful content
    int content_lines = 0;
    for (int idx = 0; idx < s->n_slides; idx++) {
        for (int li = 0; li < s->slides[idx].nlines; li++) {
            if (s->slides[idx].lines[li].type != LINE_EMPTY)
                content_lines++;
        }
    }
    if (content_lines == 0) {
        fprintf(stderr, "\033[1;33m[LINTER WARNING] %s: sin contenido de diapositivas\033[0m\n", path);
    }

    fclose(fp);
    return s;
}

static void slider_free_slide_lines(Slide *slide) {
    for (int j = 0; j < slide->nlines; j++) {
        free(slide->lines[j].cols);
        free(slide->lines[j].img_cfg);
    }
}

void slider_free(Slider *s) {
    if (!s) return;
    for (int i = 0; i < s->n_slides; i++)
        slider_free_slide_lines(&s->slides[i]);
    layouts_invalidate();
    img_cache_free_all();
    free(s);
}

int slider_get_count(Slider *s) {
    return s ? s->n_slides : 0;
}

static void theme_assign(Slider *s, const Theme *t) {
    if (t) {
        s->theme_storage = *t;
        s->theme = &s->theme_storage;
    }
}

void slider_set_theme(Slider *s, const char *theme_name) {
    if (s) theme_assign(s, theme_find(theme_name));
}

static uint32_t parse_hex_color(const char *hex) {
    if (!hex || hex[0] != '#') return 0;
    unsigned int ri, gi, bi;
    if (sscanf(hex + 1, "%2x%2x%2x", &ri, &gi, &bi) != 3) return 0;
    return (ri << 16) | (gi << 8) | bi;
}

void slider_set_color(Slider *s, const char *key, const char *hex) {
    if (!s || !key || !hex) return;
    uint32_t c = parse_hex_color(hex);
    if (!c && strcmp(hex, "#000000") != 0) return;
    if (strcmp(key, "bg") == 0) {
        s->theme_storage.bg = c;
    } else if (strcmp(key, "title") == 0) {
        s->theme_storage.title = c;
    } else if (strcmp(key, "sub") == 0 || strcmp(key, "subtitle") == 0) {
        s->theme_storage.sub = c;
    } else if (strcmp(key, "body") == 0 || strcmp(key, "text") == 0) {
        s->theme_storage.body = c;
    } else if (strcmp(key, "bullet") == 0) {
        s->theme_storage.bullet = c;
    } else if (strcmp(key, "accent") == 0) {
        s->theme_storage.accent = c;
        s->theme_storage.bullet = c;
    } else if (strcmp(key, "num") == 0 || strcmp(key, "number") == 0) {
        s->theme_storage.num = c;
    } else if (strcmp(key, "table-hdr") == 0) {
        s->theme_storage.table_hdr = c;
    } else if (strcmp(key, "table-row") == 0) {
        s->theme_storage.table_row = c;
    } else if (strcmp(key, "table-alt") == 0) {
        s->theme_storage.table_alt = c;
    } else if (strcmp(key, "table-bdr") == 0) {
        s->theme_storage.table_bdr = c;
    } else if (strcmp(key, "code-bg") == 0) {
        s->theme_storage.code_bg = c;
    } else if (strcmp(key, "code-txt") == 0) {
        s->theme_storage.code_txt = c;
    } else if (strcmp(key, "code-kw") == 0) {
        s->theme_storage.code_kw = c;
    } else if (strcmp(key, "code-com") == 0) {
        s->theme_storage.code_com = c;
    } else if (strcmp(key, "code-str") == 0) {
        s->theme_storage.code_str = c;
    } else if (strcmp(key, "code-sym") == 0) {
        s->theme_storage.code_sym = c;
    }
}

void slider_set_font_family(Slider *s, const char *font_family) {
    if (s && font_family) {
        strncpy(s->font_family, font_family, sizeof(s->font_family) - 1);
        s->font_family[sizeof(s->font_family) - 1] = '\0';
    }
}

void slider_set_font_scale(Slider *s, double font_scale) {
    if (s && font_scale > 0.1) s->font_scale = font_scale;
}

const char* slider_get_theme_name(Slider *s) {
    return (s && s->theme) ? s->theme->name : "unknown";
}

const char* slider_get_font_family(Slider *s) {
    return s ? s->font_family : "sans";
}

double slider_get_font_scale(Slider *s) {
    return s ? s->font_scale : 1.0;
}

void slider_print_notes(Slider *s, int index) {
    if (!s || index < 0 || index >= s->n_slides) return;
    Slide *slide = &s->slides[index];
    
    // Limpiar terminal usando secuencias ANSI estándar (compatible multiplataforma)
    printf("\033[H\033[2J\033[3J");
    fflush(stdout);
    
    printf("\033[1;36m=== NOTAS DEL PRESENTADOR (Slide %d / %d) ===\033[0m\n\n", index + 1, s->n_slides);
    if (strlen(slide->notes) > 0) {
        printf("%s\n", slide->notes);
    } else {
        printf("\033[3;90m(Sin notas para esta diapositiva)\033[0m\n");
    }
    printf("\n\033[1;30m----------------------------------------\033[0m\n");
    fflush(stdout);
}

// ── Test accessor functions ──────────────────────────────────────────────────

int slider_test_get_transition(Slider *s, int slide_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -1;
    return (int)s->slides[slide_idx].transition;
}

const char* slider_test_get_notes(Slider *s, int slide_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return "";
    return s->slides[slide_idx].notes;
}

int slider_test_get_nlines(Slider *s, int slide_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return 0;
    return s->slides[slide_idx].nlines;
}

int slider_test_get_line_type(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -1;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -1;
    return (int)sl->lines[line_idx].type;
}

const char* slider_test_get_line_text(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return "";
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return "";
    return sl->lines[line_idx].text;
}

const char* slider_test_get_line_marker(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return "";
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return "";
    return sl->lines[line_idx].marker;
}

int slider_test_get_line_ncols(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return 0;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return 0;
    return sl->lines[line_idx].ncols;
}

const char* slider_test_get_line_col(Slider *s, int slide_idx, int line_idx, int col_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return "";
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return "";
    SlideLine *ln = &sl->lines[line_idx];
    if (!ln->cols || col_idx < 0 || col_idx >= ln->ncols) return "";
    return ln->cols[col_idx];
}

int slider_test_get_line_has_img_cfg(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return 0;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return 0;
    return sl->lines[line_idx].img_cfg ? 1 : 0;
}

int slider_test_get_img_cfg_active(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return 0;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return 0;
    if (!sl->lines[line_idx].img_cfg) return 0;
    return sl->lines[line_idx].img_cfg->active;
}

double slider_test_get_img_cfg_opacity(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return 0.0;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return 0.0;
    if (!sl->lines[line_idx].img_cfg) return 0.0;
    return sl->lines[line_idx].img_cfg->opacity;
}

double slider_test_get_img_cfg_rotate(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return 0.0;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return 0.0;
    if (!sl->lines[line_idx].img_cfg) return 0.0;
    return sl->lines[line_idx].img_cfg->rotate;
}

int slider_test_get_img_cfg_fit(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -1;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -1;
    if (!sl->lines[line_idx].img_cfg) return -1;
    return (int)sl->lines[line_idx].img_cfg->fit;
}

int slider_test_get_img_cfg_width(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -2;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -2;
    if (!sl->lines[line_idx].img_cfg) return -2;
    return sl->lines[line_idx].img_cfg->width;
}

int slider_test_get_img_cfg_height(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -2;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -2;
    if (!sl->lines[line_idx].img_cfg) return -2;
    return sl->lines[line_idx].img_cfg->height;
}

int slider_test_get_img_cfg_width_unit(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -1;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -1;
    if (!sl->lines[line_idx].img_cfg) return -1;
    return (int)sl->lines[line_idx].img_cfg->width_unit;
}

int slider_test_get_img_cfg_height_unit(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -1;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -1;
    if (!sl->lines[line_idx].img_cfg) return -1;
    return (int)sl->lines[line_idx].img_cfg->height_unit;
}

int slider_test_get_img_cfg_radius(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -2;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -2;
    if (!sl->lines[line_idx].img_cfg) return -2;
    return sl->lines[line_idx].img_cfg->radius;
}

int slider_test_get_img_cfg_align(Slider *s, int slide_idx, int line_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return -1;
    Slide *sl = &s->slides[slide_idx];
    if (line_idx < 0 || line_idx >= sl->nlines) return -1;
    if (!sl->lines[line_idx].img_cfg) return -1;
    return (int)sl->lines[line_idx].img_cfg->align;
}

int slider_test_get_transition_type(Slider *s) {
    return s ? (int)s->transition_type : -1;
}

int slider_test_get_transition_from(Slider *s) {
    return s ? s->transition_from : -1;
}

void slider_test_set_transition_type(Slider *s, int type) {
    if (s) s->transition_type = (TransitionType)type;
}

void slider_test_set_transition_from(Slider *s, int from) {
    if (s) s->transition_from = from;
}

const char* slider_test_get_filepath(Slider *s) {
    return s ? s->filepath : "";
}

long long slider_test_get_mtime(Slider *s) {
    return s ? s->last_mtime : 0;
}

int slider_test_get_hide_num(Slider *s) {
    return s ? s->hide_num : -1;
}

int slider_test_get_kiosk_interval(Slider *s) {
    return s ? s->kiosk_interval_ms : -1;
}

int slider_test_get_line_has_anim(Slider *s, int slide_idx) {
    if (!s || slide_idx < 0 || slide_idx >= s->n_slides) return 0;
    return s->slides[slide_idx].has_anim ? 1 : 0;
}
