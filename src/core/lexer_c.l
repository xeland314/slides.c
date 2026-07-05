%{
#include "../../slider.h"
#include "highlighter.h"
#include <stdio.h>
#include <string.h>

// Temporary static variables to manage the output buffer
static char *out_buf;
static size_t out_max;
static size_t out_idx;
static const Theme *current_theme;

// Helper function to escape special HTML characters (&, <, >)
static void escape_append_flex(char c) {
  const char *esc = NULL;
  if (c == '&')
    esc = "&amp;";
  else if (c == '<')
    esc = "&lt;";
  else if (c == '>')
    esc = "&gt;";

  if (esc) {
    size_t l = strlen(esc);
    if (out_idx + l < out_max - 1) {
      memcpy(out_buf + out_idx, esc, l);
      out_idx += l;
    }
  } else {
    if (out_idx < out_max - 1) {
      out_buf[out_idx++] = c;
    }
  }
}

// Append a normal string while escaping its characters
static void append_str_flex(const char *s) {
  while (*s) {
    escape_append_flex(*s++);
  }
}

// Append a token wrapped in a Pango color tag
static void append_tagged_flex(const char *text, double r, double g, double b) {
  char tag[64];
  snprintf(tag, sizeof(tag), "<span foreground='#%02x%02x%02x'>",
           (int)(r * 255), (int)(g * 255), (int)(b * 255));

  size_t l = strlen(tag);
  if (out_idx + l < out_max - 1) {
    memcpy(out_buf + out_idx, tag, l);
    out_idx += l;
  }

  append_str_flex(text);

  if (out_idx + 7 < out_max - 1) {
    memcpy(out_buf + out_idx, "</span>", 7);
    out_idx += 7;
  }
}
%}

/* Flex options to avoid external dependencies and rename functions */
%option noyywrap
%option prefix="c_"
%option noinput nounput

/* Definiciones de expresiones regulares */
KEYWORD      "if"|"else"|"for"|"while"|"do"|"switch"|"case"|"default"|"break"|"continue"|"return"|"int"|"char"|"float"|"double"|"void"|"struct"|"typedef"|"enum"|"static"|"extern"|"const"|"inline"|"restrict"|"volatile"|"auto"|"register"|"sizeof"|"class"|"public"|"private"|"protected"|"import"|"package"|"var"|"let"|"func"|"function"|"using"|"namespace"|"try"|"catch"|"throw"|"new"|"delete"|"true"|"false"
PREPROC      "#"[a-zA-Z_0-9]+
IDENTIFIER   [a-zA-Z_][a-zA-Z0-9_]*
STRING       "\""([^"\\\n]|\\.)*"\""|"'"([^'\\\n]|\\.)*"'"
COMMENT      "//".*

%%

{KEYWORD}    { append_tagged_flex(c_text, current_theme->code_kw_r, current_theme->code_kw_g, current_theme->code_kw_b); }
{PREPROC}    { append_tagged_flex(c_text, current_theme->code_kw_r, current_theme->code_kw_g, current_theme->code_kw_b); }
{COMMENT}    { append_tagged_flex(c_text, current_theme->code_com_r, current_theme->code_com_g, current_theme->code_com_b); }
{STRING}     { append_tagged_flex(c_text, current_theme->code_str_r, current_theme->code_str_g, current_theme->code_str_b); }
{IDENTIFIER} { append_str_flex(c_text); }
[{}()\[\]]   { append_tagged_flex(c_text, current_theme->code_sym_r, current_theme->code_sym_g, current_theme->code_sym_b); }
.            { escape_append_flex(c_text[0]); }

%%

void c_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = c__scan_string(line);
  c_lex();
  c__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
