%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

/* Flex options to avoid external dependencies and rename functions */
%option noyywrap
%option prefix="go_"
%option noinput nounput

/* NUMBER DEFINITIONS (With support for underscores as separators) */
DIGIT          [0-9]
NUM_INT        {DIGIT}(_?{DIGIT})*
NUM_HEX        0[xX][0-9a-fA-F](_?[0-9a-fA-F])*
NUM_BIN        0[bB][01](_?[01])*
NUM_OCT        0[oO]?[0-7](_?[0-7])*

/* Floating point and Scientific Notation */
NUM_FLOAT      ({NUM_INT}\.{NUM_INT}?|\.{NUM_INT})([eE][+-]?{NUM_INT})?|{NUM_INT}[eE][+-]?{NUM_INT}

/* Go type suffix for complex */
GO_COMPLEX     [iI]

/* Other definitions */
KEYWORD      "func"|"package"|"import"|"var"|"const"|"type"|"struct"|"interface"|"map"|"chan"|"go"|"defer"|"select"|"case"|"switch"|"if"|"else"|"for"|"range"|"return"|"break"|"continue"|"fallthrough"|"nil"|"true"|"false"|"iota"|"make"|"new"|"len"|"cap"|"append"|"copy"|"delete"|"panic"|"recover"|"print"|"println"
BUILTIN      "int"|"int8"|"int16"|"int32"|"int64"|"uint"|"uint8"|"uint16"|"uint32"|"uint64"|"float32"|"float64"|"complex64"|"complex128"|"bool"|"byte"|"rune"|"string"|"error"|"any"|"comparable"
IDENTIFIER   [a-zA-Z_][a-zA-Z0-9_]*
COMMENT      "//".*
STRING       "\""([^"\\\n]|\\.)*"\""|"'"([^'\\\n]|\\.)*"'"

%%

{KEYWORD}    { append_tagged_flex(go_text, current_theme->code_kw); }
{BUILTIN}    { append_tagged_flex(go_text, current_theme->code_kw); }
{COMMENT}    { append_tagged_flex(go_text, current_theme->code_com); }
{STRING}     { append_tagged_flex(go_text, current_theme->code_str); }
{NUM_HEX}|{NUM_BIN}|{NUM_OCT}|{NUM_INT} { append_tagged_flex(go_text, current_theme->code_sym); }
{NUM_FLOAT}{GO_COMPLEX}?                { append_tagged_flex(go_text, current_theme->code_sym); }
{IDENTIFIER} { append_str_flex(go_text); }
[{}()\[\]]   { append_tagged_flex(go_text, current_theme->code_sym); }
[-+*/%=<>!&|^~:] { append_tagged_flex(go_text, current_theme->code_sym); }
.            { escape_append_flex(go_text[0]); }

%%

void go_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = go__scan_string(line);
  go_lex();
  go__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
