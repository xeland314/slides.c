#include "../../slider.h"
#include "internal.h"
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
        fclose(fp);
        return NULL;
    }

    Slider *s = calloc(1, sizeof(Slider));
    if (!s) { fclose(fp); return NULL; }
    s->theme = theme_default();
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
                        s->theme = theme_find(val);
                    } else if (strcmp(key, "font") == 0 || strcmp(key, "font-family") == 0) {
                        strncpy(s->font_family, val, sizeof(s->font_family) - 1);
                        s->font_family[sizeof(s->font_family) - 1] = '\0';
                    } else if (strcmp(key, "scale") == 0 || strcmp(key, "font-scale") == 0) {
                        double sc = atof(val);
                        if (sc > 0.1) s->font_scale = sc;
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

    fclose(fp);
    return s;
}

void slider_free(Slider *s) {
    if (s) free(s);
}

int slider_get_count(Slider *s) {
    return s ? s->n_slides : 0;
}

void slider_set_theme(Slider *s, const char *theme_name) {
    if (s) s->theme = theme_find(theme_name);
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
