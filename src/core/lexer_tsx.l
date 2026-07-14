%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

%option noyywrap
%option prefix="tsx_"
%option noinput nounput

/* ── Definitions (sync with ts_defs.l) ── */
DIGIT          [0-9]
NUM_INT        {DIGIT}(_?{DIGIT})*
NUM_HEX        0[xX][0-9a-fA-F](_?[0-9a-fA-F])*
NUM_BIN        0[bB][01](_?[01])*
NUM_OCT        0[oO]?[0-7](_?[0-7])*
NUM_FLOAT      ({NUM_INT}\.{NUM_INT}?|\.{NUM_INT})([eE][+-]?{NUM_INT})?|{NUM_INT}[eE][+-]?{NUM_INT}

/* JavaScript base keywords */
KEYWORD_JS     "var"|"let"|"const"|"function"|"return"|"if"|"else"|"for"|"while"|"do"|"switch"|"case"|"default"|"break"|"continue"|"new"|"this"|"class"|"extends"|"super"|"import"|"export"|"from"|"as"|"try"|"catch"|"finally"|"throw"|"async"|"await"|"yield"|"of"|"in"|"instanceof"|"typeof"|"void"|"delete"|"debugger"|"with"|"static"|"get"|"set"|"true"|"false"|"null"|"undefined"|"NaN"|"Infinity"

/* TypeScript-specific keywords */
KEYWORD_TS     "type"|"interface"|"enum"|"namespace"|"module"|"declare"|"abstract"|"implements"|"readonly"|"private"|"protected"|"public"|"keyof"|"infer"|"is"|"asserts"|"satisfies"|"override"

/* Combined keywords */
KEYWORD        {KEYWORD_JS}|{KEYWORD_TS}

IDENTIFIER     [a-zA-Z_$][a-zA-Z0-9_$]*
COMMENT_SINGLE "//".*
COMMENT_MULTI  "/*"([^*]|"*"+[^*/])*"*"+"/"
STRING_DQ      "\""([^"\\\n]|\\.)*"\""
STRING_SQ      "'"([^'\\\n]|\\.)*"'"
STRING_BT      "`"([^`\\\n]|\\.)*"`"

/* JSX */
JSX_TAG_NAME   [a-zA-Z][a-zA-Z0-9_.-]*
JSX_ATTR       [a-zA-Z_:][a-zA-Z0-9_:]*

%x JSX_TAG JSX_CHILD

%%

  /* ── Initial state: code ── */

{KEYWORD}        { append_tagged_flex(tsx_text, current_theme->code_kw); }
{COMMENT_SINGLE} { append_tagged_flex(tsx_text, current_theme->code_com); }
{COMMENT_MULTI}  { append_tagged_flex(tsx_text, current_theme->code_com); }
{STRING_DQ}      { append_tagged_flex(tsx_text, current_theme->code_str); }
{STRING_SQ}      { append_tagged_flex(tsx_text, current_theme->code_str); }
{STRING_BT}      { append_tagged_flex(tsx_text, current_theme->code_str); }
{NUM_HEX}|{NUM_BIN}|{NUM_OCT}|{NUM_INT} { append_tagged_flex(tsx_text, current_theme->code_sym); }
{NUM_FLOAT}                                    { append_tagged_flex(tsx_text, current_theme->code_sym); }
{IDENTIFIER}   { append_str_flex(tsx_text); }
[{}()\[\]]     { append_tagged_flex(tsx_text, current_theme->code_sym); }

  /* JSX open tag */
"</"{JSX_TAG_NAME} {
    BEGIN(JSX_TAG);
    escape_append_flex('<');
    escape_append_flex('/');
    const char *tag = tsx_text + 2;
    append_tagged_flex(tag, current_theme->code_kw);
  }

"<"{JSX_TAG_NAME} {
    BEGIN(JSX_TAG);
    escape_append_flex('<');
    const char *tag = tsx_text + 1;
    append_tagged_flex(tag, current_theme->code_kw);
  }

"<!"             { append_tagged_flex(tsx_text, current_theme->code_sym); }

  /* ── JSX_TAG state ── */

<JSX_TAG>"/>"  {
    BEGIN(INITIAL);
    escape_append_flex('/');
    escape_append_flex('>');
  }

<JSX_TAG>">"   {
    BEGIN(JSX_CHILD);
    escape_append_flex('>');
  }

<JSX_TAG>{JSX_ATTR}"=" {
    /* attribute name */
    size_t nlen = tsx_leng - 1;
    char *attr = (char *)malloc(nlen + 1);
    if (attr) {
      memcpy(attr, tsx_text, nlen);
      attr[nlen] = '\0';
      append_tagged_flex(attr, current_theme->code_kw);
      free(attr);
    }
    escape_append_flex('=');
  }

<JSX_TAG>{STRING_DQ} { append_tagged_flex(tsx_text, current_theme->code_str); }
<JSX_TAG>{STRING_SQ} { append_tagged_flex(tsx_text, current_theme->code_str); }

<JSX_TAG>"{"[^\}]*"}" {
    /* attribute expression: {expr} */
    append_tagged_flex(tsx_text, current_theme->code_kw);
  }

<JSX_TAG>[a-zA-Z_:][a-zA-Z0-9_:.]* {
    append_tagged_flex(tsx_text, current_theme->code_kw);
  }

<JSX_TAG>[ \t\r]+  { append_str_flex(tsx_text); }
<JSX_TAG>.         { escape_append_flex(tsx_text[0]); }

  /* ── JSX_CHILD state ── */

<JSX_CHILD>"</" {
    BEGIN(JSX_TAG);
    escape_append_flex('<');
    escape_append_flex('/');
  }

<JSX_CHILD>"<" {
    BEGIN(JSX_TAG);
    escape_append_flex('<');
  }

<JSX_CHILD>"{" {
    append_tagged_flex(tsx_text, current_theme->code_sym);
  }

<JSX_CHILD>"}" {
    append_tagged_flex(tsx_text, current_theme->code_sym);
  }

<JSX_CHILD>{STRING_DQ} { append_tagged_flex(tsx_text, current_theme->code_str); }
<JSX_CHILD>{STRING_SQ} { append_tagged_flex(tsx_text, current_theme->code_str); }

<JSX_CHILD>[^<{\"']+/[^<{\"'] {
    /* plain text between tags: match runs of non-special chars */
    append_str_flex(tsx_text);
  }

<JSX_CHILD>[ \t\r]+  { append_str_flex(tsx_text); }
<JSX_CHILD>.          { escape_append_flex(tsx_text[0]); }

  /* ── Fallback ── */
[-+*/%=<>!&|^~?:.] { append_tagged_flex(tsx_text, current_theme->code_sym); }
.                  { escape_append_flex(tsx_text[0]); }

%%

void tsx_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = tsx__scan_string(line);
  BEGIN(INITIAL);
  tsx_lex();
  tsx__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
