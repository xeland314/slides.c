#include "highlighter.h"
#include <stdlib.h>

void c_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size);

void highlighter_highlight(const char *line, const Theme *theme, char *out, size_t out_size) {
    if (out == NULL || out_size == 0) {
        return;
    }

    out[0] = '\0';
    if (line == NULL) {
        return;
    }

    c_lexer_run(line, theme, out, out_size);
}
