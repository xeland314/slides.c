#include "highlighter.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static const char *keywords[] = {
    "if", "else", "for", "while", "do", "switch", "case", "default",
    "break", "continue", "return", "int", "char", "float", "double",
    "void", "struct", "typedef", "enum", "static", "extern", "const",
    "inline", "restrict", "volatile", "auto", "register", "sizeof",
    "class", "public", "private", "protected", "import", "package",
    "include", "define", "var", "let", "func", "function", "using",
    "namespace", "try", "catch", "throw", "new", "delete", "true", "false",
    NULL
};

static int is_keyword(const char *word) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

static void escape_append(char *out, size_t *wi, size_t out_size, char c) {
    const char *esc = NULL;
    if (c == '&') esc = "&amp;";
    else if (c == '<') esc = "&lt;";
    else if (c == '>') esc = "&gt;";

    if (esc) {
        size_t l = strlen(esc);
        if (*wi + l < out_size - 1) {
            memcpy(out + *wi, esc, l);
            *wi += l;
        }
    } else {
        if (*wi < out_size - 1) {
            out[*wi] = c;
            (*wi)++;
        }
    }
}

static void append_str(char *out, size_t *wi, size_t out_size, const char *s) {
    size_t l = strlen(s);
    if (*wi + l < out_size - 1) {
        memcpy(out + *wi, s, l);
        *wi += l;
    }
}

static void append_color_tag(char *out, size_t *wi, size_t out_size, double r, double g, double b) {
    char buf[64];
    snprintf(buf, sizeof(buf), "<span foreground='#%02x%02x%02x'>", 
             (int)(r * 255), (int)(g * 255), (int)(b * 255));
    append_str(out, wi, out_size, buf);
}

void highlighter_highlight(const char *line, const Theme *theme, char *out, size_t out_size) {
    size_t wi = 0;
    const char *p = line;
    
    while (*p) {
        // Comments
        if (p[0] == '/' && p[1] == '/') {
            append_color_tag(out, &wi, out_size, theme->code_com_r, theme->code_com_g, theme->code_com_b);
            while (*p) {
                escape_append(out, &wi, out_size, *p);
                p++;
            }
            append_str(out, &wi, out_size, "</span>");
            break;
        }

        // Strings
        if (*p == '"' || *p == '\'') {
            char quote = *p;
            append_color_tag(out, &wi, out_size, theme->code_str_r, theme->code_str_g, theme->code_str_b);
            escape_append(out, &wi, out_size, *p++);
            while (*p && *p != quote) {
                if (*p == '\\' && *(p+1)) {
                    escape_append(out, &wi, out_size, *p++);
                }
                escape_append(out, &wi, out_size, *p++);
            }
            if (*p == quote) escape_append(out, &wi, out_size, *p++);
            append_str(out, &wi, out_size, "</span>");
            continue;
        }

        // Keywords and Identifiers
        if (isalpha(*p) || *p == '_' || *p == '#') {
            char word[128];
            int i = 0;
            while (*p && (isalnum(*p) || *p == '_' || *p == '#') && i < 127) {
                word[i++] = *p++;
            }
            word[i] = '\0';

            if (is_keyword(word) || word[0] == '#') {
                append_color_tag(out, &wi, out_size, theme->code_kw_r, theme->code_kw_g, theme->code_kw_b);
                append_str(out, &wi, out_size, word);
                append_str(out, &wi, out_size, "</span>");
            } else {
                // Regular word, but need to escape it
                for(int j=0; j<i; j++) escape_append(out, &wi, out_size, word[j]);
            }
            continue;
        }

        // Symbols
        if (strchr("{}[]()", *p)) {
            append_color_tag(out, &wi, out_size, theme->code_sym_r, theme->code_sym_g, theme->code_sym_b);
            escape_append(out, &wi, out_size, *p++);
            append_str(out, &wi, out_size, "</span>");
            continue;
        }

        // Other chars
        escape_append(out, &wi, out_size, *p++);
    }
    out[wi] = '\0';
}
