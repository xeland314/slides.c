%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

/* Flex options to avoid external dependencies and rename functions */
%option noyywrap
%option prefix="c_"
%option noinput nounput

/* NUMBER DEFINITIONS (With support for underscores as separators) */
DIGIT          [0-9]
NUM_INT        {DIGIT}(_?{DIGIT})*
NUM_HEX        0[xX][0-9a-fA-F](_?[0-9a-fA-F])*
NUM_BIN        0[bB][01](_?[01])*
NUM_OCT        0[oO]?[0-7](_?[0-7])*

/* Floating point and Scientific Notation */
NUM_FLOAT      ({NUM_INT}\.{NUM_INT}?|\.{NUM_INT})([eE][+-]?{NUM_INT})?|{NUM_INT}[eE][+-]?{NUM_INT}

/* C suffixes */
C_INT_SUF      ([uU][lL]{0,2}|[lL]{0,2}[uU]?)
C_FLT_SUF      [fFlL]

/* Other definitions */
KEYWORD      "if"|"else"|"for"|"while"|"do"|"switch"|"case"|"default"|"break"|"continue"|"return"|"int"|"char"|"float"|"double"|"void"|"struct"|"typedef"|"enum"|"static"|"extern"|"const"|"inline"|"sizeof"
PREPROC      "#"[a-zA-Z_0-9]+
IDENTIFIER   [a-zA-Z_][a-zA-Z0-9_]*
STRING       "\""([^"\\\n]|\\.)*"\""|"'"([^'\\\n]|\\.)*"'"
COMMENT      "//".*

%%

{KEYWORD}    { append_tagged_flex(c_text, current_theme->code_kw); }
{PREPROC}    { append_tagged_flex(c_text, current_theme->code_kw); }
{COMMENT}    { append_tagged_flex(c_text, current_theme->code_com); }
{STRING}     { append_tagged_flex(c_text, current_theme->code_str); }
({NUM_HEX}|{NUM_BIN}|{NUM_OCT}|{NUM_INT}){C_INT_SUF}? { append_tagged_flex(c_text, current_theme->code_sym); }
{NUM_FLOAT}{C_FLT_SUF}?                               { append_tagged_flex(c_text, current_theme->code_sym); }
{IDENTIFIER} { append_str_flex(c_text); }
[{}()\[\]]   { append_tagged_flex(c_text, current_theme->code_sym); }
[-+*/%=<>!&|^~] { append_tagged_flex(c_text, current_theme->code_sym); }
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
