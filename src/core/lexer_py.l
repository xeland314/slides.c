%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

/* Flex options to avoid external dependencies and rename functions */
%option noyywrap
%option prefix="py_"
%option noinput nounput

/* NUMBER DEFINITIONS */
DIGIT          [0-9]
NUM_INT        {DIGIT}(_?{DIGIT})*
NUM_HEX        0[xX][0-9a-fA-F](_?[0-9a-fA-F])*
NUM_BIN        0[bB][01](_?[01])*
NUM_OCT        0[oO][0-7](_?[0-7])*

NUM_FLOAT      ({NUM_INT}\.{NUM_INT}?|\.{NUM_INT})([eE][+-]?{NUM_INT})?|{NUM_INT}[eE][+-]?{NUM_INT}

/* Python Complex Suffix */
PY_COM_SUF     [jJ]

KEYWORD      "def"|"class"|"return"|"if"|"elif"|"else"|"for"|"while"|"import"|"from"|"as"|"in"|"is"|"not"|"and"|"or"|"try"|"except"|"raise"|"with"|"lambda"|"pass"|"break"|"continue"|"None"|"True"|"False"
IDENTIFIER   [a-zA-Z_][a-zA-Z0-9_]*
COMMENT      "#".*
STRING       "\""([^"\\\n]|\\.)*"\""|"'"([^'\\\n]|\\.)*"'"

%%

{KEYWORD}    { append_tagged_flex(py_text, current_theme->code_kw_r, current_theme->code_kw_g, current_theme->code_kw_b); }
{COMMENT}    { append_tagged_flex(py_text, current_theme->code_com_r, current_theme->code_com_g, current_theme->code_com_b); }
{STRING}     { append_tagged_flex(py_text, current_theme->code_str_r, current_theme->code_str_g, current_theme->code_str_b); }
{NUM_HEX}|{NUM_BIN}|{NUM_OCT}|{NUM_INT} { append_tagged_flex(py_text, current_theme->code_sym_r, current_theme->code_sym_g, current_theme->code_sym_b); }
{NUM_FLOAT}{PY_COM_SUF}?                { append_tagged_flex(py_text, current_theme->code_sym_r, current_theme->code_sym_g, current_theme->code_sym_b); }
{IDENTIFIER} { append_str_flex(py_text); }
[{}()\[\]]   { append_tagged_flex(py_text, current_theme->code_sym_r, current_theme->code_sym_g, current_theme->code_sym_b); }
[-+*/%=<>!&|^~] { append_tagged_flex(py_text, current_theme->code_sym_r, current_theme->code_sym_g, current_theme->code_sym_b); }
.            { escape_append_flex(py_text[0]); }

%%

void py_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = py__scan_string(line);
  py_lex();
  py__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
