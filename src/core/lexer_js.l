%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

%option noyywrap
%option prefix="js_"
%option noinput nounput

/* ── Definitions (sync with ts_defs.l) ── */
DIGIT          [0-9]
NUM_INT        {DIGIT}(_?{DIGIT})*
NUM_HEX        0[xX][0-9a-fA-F](_?[0-9a-fA-F])*
NUM_BIN        0[bB][01](_?[01])*
NUM_OCT        0[oO]?[0-7](_?[0-7])*
NUM_FLOAT      ({NUM_INT}\.{NUM_INT}?|\.{NUM_INT})([eE][+-]?{NUM_INT})?|{NUM_INT}[eE][+-]?{NUM_INT}

/* JavaScript keywords */
KEYWORD        "var"|"let"|"const"|"function"|"return"|"if"|"else"|"for"|"while"|"do"|"switch"|"case"|"default"|"break"|"continue"|"new"|"this"|"class"|"extends"|"super"|"import"|"export"|"from"|"as"|"try"|"catch"|"finally"|"throw"|"async"|"await"|"yield"|"of"|"in"|"instanceof"|"typeof"|"void"|"delete"|"debugger"|"with"|"static"|"get"|"set"|"true"|"false"|"null"|"undefined"|"NaN"|"Infinity"

IDENTIFIER     [a-zA-Z_$][a-zA-Z0-9_$]*
COMMENT_SINGLE "//".*
COMMENT_MULTI  "/*"([^*]|"*"+[^*/])*"*"+"/"
STRING_DQ      "\""([^"\\\n]|\\.)*"\""
STRING_SQ      "'"([^'\\\n]|\\.)*"'"
STRING_BT      "`"([^`\\\n]|\\.)*"`"

%%

{KEYWORD}        { append_tagged_flex(js_text, current_theme->code_kw); }
{COMMENT_SINGLE} { append_tagged_flex(js_text, current_theme->code_com); }
{COMMENT_MULTI}  { append_tagged_flex(js_text, current_theme->code_com); }
{STRING_DQ}      { append_tagged_flex(js_text, current_theme->code_str); }
{STRING_SQ}      { append_tagged_flex(js_text, current_theme->code_str); }
{STRING_BT}      { append_tagged_flex(js_text, current_theme->code_str); }
{NUM_HEX}|{NUM_BIN}|{NUM_OCT}|{NUM_INT} { append_tagged_flex(js_text, current_theme->code_sym); }
{NUM_FLOAT}                                    { append_tagged_flex(js_text, current_theme->code_sym); }
{IDENTIFIER}   { append_str_flex(js_text); }
[{}()\[\]]     { append_tagged_flex(js_text, current_theme->code_sym); }
[-+*/%=<>!&|^~?:] { append_tagged_flex(js_text, current_theme->code_sym); }
.              { escape_append_flex(js_text[0]); }

%%

void js_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = js__scan_string(line);
  js_lex();
  js__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
