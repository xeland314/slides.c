#include "highlighter.h"
#include <string.h>

void c_lexer_run(const char *line, const Theme *theme, char *out,
                 size_t out_size);
void py_lexer_run(const char *line, const Theme *theme, char *out,
                  size_t out_size);
void go_lexer_run(const char *line, const Theme *theme, char *out,
                  size_t out_size);
void js_lexer_run(const char *line, const Theme *theme, char *out,
                  size_t out_size);
void ts_lexer_run(const char *line, const Theme *theme, char *out,
                  size_t out_size);
void tsx_lexer_run(const char *line, const Theme *theme, char *out,
                   size_t out_size);
void html_lexer_run(const char *line, const Theme *theme, char *out,
                    size_t out_size);
void css_lexer_run(const char *line, const Theme *theme, char *out,
                   size_t out_size);
void sh_lexer_run(const char *line, const Theme *theme, char *out,
                  size_t out_size);

void highlighter_highlight(const char *line, const char *lang,
                           const Theme *theme, char *out, size_t out_size) {
  if (!out || out_size == 0)
    return;
  out[0] = '\0';
  if (!line)
    return;

  // Python family: python, py
  if (lang && (strcmp(lang, "python") == 0 || strcmp(lang, "py") == 0)) {
    py_lexer_run(line, theme, out, out_size);
  // Go
  } else if (lang && strcmp(lang, "go") == 0) {
    go_lexer_run(line, theme, out, out_size);
  // JavaScript family: javascript, js
  } else if (lang && (strcmp(lang, "javascript") == 0 || strcmp(lang, "js") == 0)) {
    js_lexer_run(line, theme, out, out_size);
  // TypeScript family: typescript, ts
  } else if (lang && (strcmp(lang, "typescript") == 0 || strcmp(lang, "ts") == 0)) {
    ts_lexer_run(line, theme, out, out_size);
  // TSX/JSX family: tsx, jsx
  } else if (lang && (strcmp(lang, "tsx") == 0 || strcmp(lang, "jsx") == 0)) {
    tsx_lexer_run(line, theme, out, out_size);
  // HTML
  } else if (lang && strcmp(lang, "html") == 0) {
    html_lexer_run(line, theme, out, out_size);
  // CSS
  } else if (lang && strcmp(lang, "css") == 0) {
    css_lexer_run(line, theme, out, out_size);
  // Shell family: shell, bash, sh, fish, zsh, env
  } else if (lang && (strcmp(lang, "shell") == 0 || strcmp(lang, "bash") == 0 ||
                      strcmp(lang, "sh") == 0 || strcmp(lang, "fish") == 0 ||
                      strcmp(lang, "zsh") == 0 || strcmp(lang, "env") == 0)) {
    sh_lexer_run(line, theme, out, out_size);
  // C family: c, cpp, c++, java, csharp, cs, c#, kotlin, kt, swift, dart, rust, rs, php, scala
  } else if (lang && (strcmp(lang, "c") == 0 || strcmp(lang, "cpp") == 0 ||
                      strcmp(lang, "c++") == 0 || strcmp(lang, "java") == 0 ||
                      strcmp(lang, "csharp") == 0 || strcmp(lang, "cs") == 0 ||
                      strcmp(lang, "c#") == 0 || strcmp(lang, "kotlin") == 0 ||
                      strcmp(lang, "kt") == 0 || strcmp(lang, "swift") == 0 ||
                      strcmp(lang, "dart") == 0 || strcmp(lang, "rust") == 0 ||
                      strcmp(lang, "rs") == 0 || strcmp(lang, "php") == 0 ||
                      strcmp(lang, "scala") == 0)) {
    c_lexer_run(line, theme, out, out_size);
  } else {
    // Por defecto usamos C
    c_lexer_run(line, theme, out, out_size);
  }
}
