#include "../../slider.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

Slider* slider_load(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) { fprintf(stderr, "No se pudo abrir: %s\n", path); return NULL; }

    Slider *s = calloc(1, sizeof(Slider));
    if (!s) { fclose(fp); return NULL; }

    int n = 0;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp) && n < MAX_SLIDES) {
        // Separador de slide
        char trimmed[MAX_LINE_LEN];
        strncpy(trimmed, line, MAX_LINE_LEN - 1);
        char *t = trim(trimmed);
        if (strcmp(t, "---") == 0) {
            n++;
            continue;
        }
        Slide *cur = &s->slides[n];
        if (cur->nlines < MAX_LINES) {
            parse_line(line, &cur->lines[cur->nlines]);
            cur->nlines++;
        }
    }
    s->n_slides = n + 1;
    fclose(fp);
    return s;
}

void slider_free(Slider *s) {
    if (s) free(s);
}

int slider_get_count(Slider *s) {
    return s ? s->n_slides : 0;
}
