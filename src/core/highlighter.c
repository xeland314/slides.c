#include "highlighter.h"
#include <string.h>

void c_lexer_run(const char *line, const Theme *theme, char *out,
                 size_t out_size);
void py_lexer_run(const char *line, const Theme *theme, char *out,
                  size_t out_size);

void highlighter_highlight(const char *line, const char *lang,
                           const Theme *theme, char *out, size_t out_size) {
  if (!out || out_size == 0)
    return;
  out[0] = '\0';
  if (!line)
    return;

  if (lang && strcmp(lang, "python") == 0) {
    py_lexer_run(line, theme, out, out_size);
  } else {
    // Por defecto usamos C
    c_lexer_run(line, theme, out, out_size);
  }
}
