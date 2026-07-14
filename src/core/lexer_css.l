%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

%option noyywrap
%option prefix="css_"
%option noinput nounput

/* ── Definitions ── */
DIGIT        [0-9]
NUM_INT      {DIGIT}+
NUM_FLOAT    {DIGIT}+"."{DIGIT}*
NUM_SIGNED   [-+]?{NUM_FLOAT}|[-+]?{NUM_INT}

/* CSS units */
UNIT         "px"|"em"|"rem"|"%"|"vh"|"vw"|"vmin"|"vmax"|"ch"|"ex"|"cm"|"mm"|"in"|"pt"|"pc"|"s"|"ms"|"deg"|"rad"|"grad"|"turn"|"fr"

/* Number with optional unit */
CSS_NUM      {NUM_SIGNED}{UNIT}?

IDENTIFIER   [a-zA-Z_-][a-zA-Z0-9_-]*
STRING_DQ    "\""([^"\\\n]|\\.)*"\""
STRING_SQ    "'"([^'\\\n]|\\.)*"'"
COMMENT      "/*"([^*]|"*"+[^*/])*"*"+"/"

/* Hash color */
HASH_COLOR   "#"[0-9a-fA-F]{3,8}

/* Selector characters */
SEL_CLASS    "."{IDENTIFIER}
SEL_ID       "#"{IDENTIFIER}
SEL_PSEUDO   ":"{1,2}{IDENTIFIER}

/* At-rules */
AT_RULE      "@"[a-zA-Z-]+

/* Property (common, used as fallback) */
PROPERTY     [a-zA-Z-]+

%%

{COMMENT}        { append_tagged_flex(css_text, current_theme->code_com); }
{STRING_DQ}      { append_tagged_flex(css_text, current_theme->code_str); }
{STRING_SQ}      { append_tagged_flex(css_text, current_theme->code_str); }

  /* At-rules */
{AT_RULE}        { append_tagged_flex(css_text, current_theme->code_kw); }

  /* Selectors */
{SEL_ID}         { append_tagged_flex(css_text, current_theme->code_kw); }
{SEL_CLASS}      { append_tagged_flex(css_text, current_theme->code_kw); }
{SEL_PSEUDO}     { append_tagged_flex(css_text, current_theme->code_sym); }

  /* Hash color */
{HASH_COLOR}     { append_tagged_flex(css_text, current_theme->code_str); }

  /* Numbers with units */
{CSS_NUM}        { append_tagged_flex(css_text, current_theme->code_sym); }

  /* Important */
"!important"     { append_tagged_flex(css_text, current_theme->code_kw); }

  /* Property-like identifiers followed by colon */
{IDENTIFIER}":"  {
    /* property: value pattern */
    size_t plen = css_leng - 1;
    char *prop = (char *)malloc(plen + 1);
    if (prop) {
      memcpy(prop, css_text, plen);
      prop[plen] = '\0';
      append_tagged_flex(prop, current_theme->code_kw);
      free(prop);
    }
    escape_append_flex(':');
  }

  /* Bare identifiers (tag selectors, values like inherit/none/auto, etc.) */
{IDENTIFIER}     { append_str_flex(css_text); }

  /* Selectors and combinators */
[{}();:,]        { append_tagged_flex(css_text, current_theme->code_sym); }
[>+~*]           { append_tagged_flex(css_text, current_theme->code_sym); }
"="              { append_tagged_flex(css_text, current_theme->code_sym); }

[ \t\r]+         { append_str_flex(css_text); }
.                { escape_append_flex(css_text[0]); }

%%

void css_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = css__scan_string(line);
  css_lex();
  css__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
