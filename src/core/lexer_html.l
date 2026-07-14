%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

%option noyywrap
%option prefix="html_"
%option noinput nounput

/* ── Definitions ── */
IDENTIFIER   [a-zA-Z][a-zA-Z0-9_-]*
STRING_DQ    "\""([^"\\\n]|\\.)*"\""
STRING_SQ    "'"([^'\\\n]|\\.)*"'"
COMMENT      "<!--"([^\-]|"-"[^\-]|"--"[^>])*"-->"
ENTITY       "&"[a-zA-Z]+";"
WHITESPACE   [ \t\r\n]+

/* HTML tag names (common) */
TAG          "div"|"span"|"p"|"a"|"h1"|"h2"|"h3"|"h4"|"h5"|"h6"|"ul"|"ol"|"li"|"table"|"tr"|"td"|"th"|"thead"|"tbody"|"form"|"input"|"button"|"select"|"option"|"textarea"|"img"|"br"|"hr"|"meta"|"link"|"script"|"style"|"head"|"body"|"html"|"title"|"section"|"article"|"nav"|"main"|"footer"|"header"|"aside"|"figure"|"figcaption"|"video"|"audio"|"source"|"canvas"|"svg"|"path"|"iframe"|"pre"|"code"|"strong"|"em"|"small"|"i"|"b"|"u"|"s"|"sub"|"sup"|"label"|"fieldset"|"legend"|"datalist"|"output"|"progress"|"meter"|"details"|"summary"|"dialog"|"template"|"slot"

/* Common attributes */
ATTR         "class"|"id"|"style"|"href"|"src"|"alt"|"title"|"name"|"type"|"value"|"placeholder"|"action"|"method"|"target"|"rel"|"charset"|"content"|"lang"|"dir"|"tabindex"|"hidden"|"disabled"|"checked"|"required"|"readonly"|"multiple"|"autofocus"|"autocomplete"|"max"|"min"|"step"|"pattern"|"accept"|"colspan"|"rowspan"|"scope"|"role"|"aria-"[a-z-]+|"data-"[a-z-]+

%x TAG_OPEN TAG_CLOSE DOCTYPE

%%

  /* ── Initial state: text content ── */

{COMMENT}      { append_tagged_flex(html_text, current_theme->code_com); }

{ENTITY}       { append_tagged_flex(html_text, current_theme->code_sym); }

"<!"           {
    BEGIN(DOCTYPE);
    append_tagged_flex(html_text, current_theme->code_kw);
  }

"</"           {
    BEGIN(TAG_CLOSE);
    escape_append_flex('<');
    escape_append_flex('/');
  }

"<"{TAG}       {
    BEGIN(TAG_OPEN);
    escape_append_flex('<');
    const char *tag = html_text + 1;
    append_tagged_flex(tag, current_theme->code_kw);
  }

"<"{IDENTIFIER} {
    BEGIN(TAG_OPEN);
    escape_append_flex('<');
    const char *tag = html_text + 1;
    append_tagged_flex(tag, current_theme->code_kw);
  }

"<"            { append_tagged_flex(html_text, current_theme->code_sym); }
">"            { append_tagged_flex(html_text, current_theme->code_sym); }
"&"            { escape_append_flex(html_text[0]); }

{IDENTIFIER}   { append_str_flex(html_text); }
{WHITESPACE}   { append_str_flex(html_text); }
.              { escape_append_flex(html_text[0]); }

  /* ── TAG_OPEN state: inside opening tag ── */

<TAG_OPEN>"/>" {
    BEGIN(INITIAL);
    escape_append_flex('/');
    escape_append_flex('>');
  }

<TAG_OPEN>">"  {
    BEGIN(INITIAL);
    escape_append_flex('>');
  }

<TAG_OPEN>{ATTR}"="{STRING_DQ} {
    /* attribute="value" */
    size_t eq = 0;
    while (eq < html_leng && html_text[eq] != '=') eq++;
    char *attr = (char *)malloc(eq + 1);
    if (attr) {
      memcpy(attr, html_text, eq);
      attr[eq] = '\0';
      append_tagged_flex(attr, current_theme->code_kw);
      free(attr);
    }
    escape_append_flex('=');
    /* value part */
    size_t vstart = eq + 1;
    size_t vlen = html_leng - vstart;
    char *val = (char *)malloc(vlen + 1);
    if (val) {
      memcpy(val, html_text + vstart, vlen);
      val[vlen] = '\0';
      append_tagged_flex(val, current_theme->code_str);
      free(val);
    }
  }

<TAG_OPEN>{ATTR}"="{STRING_SQ} {
    size_t eq = 0;
    while (eq < html_leng && html_text[eq] != '=') eq++;
    char *attr = (char *)malloc(eq + 1);
    if (attr) {
      memcpy(attr, html_text, eq);
      attr[eq] = '\0';
      append_tagged_flex(attr, current_theme->code_kw);
      free(attr);
    }
    escape_append_flex('=');
    size_t vstart = eq + 1;
    size_t vlen = html_leng - vstart;
    char *val = (char *)malloc(vlen + 1);
    if (val) {
      memcpy(val, html_text + vstart, vlen);
      val[vlen] = '\0';
      append_tagged_flex(val, current_theme->code_str);
      free(val);
    }
  }

<TAG_OPEN>{ATTR} {
    append_tagged_flex(html_text, current_theme->code_kw);
  }

<TAG_OPEN>{IDENTIFIER} {
    append_str_flex(html_text);
  }

<TAG_OPEN>[ \t\r]+ { append_str_flex(html_text); }
<TAG_OPEN>.         { escape_append_flex(html_text[0]); }

  /* ── TAG_CLOSE state: inside closing tag ── */

<TAG_CLOSE>">"  {
    BEGIN(INITIAL);
    escape_append_flex('>');
  }

<TAG_CLOSE>{TAG} {
    append_tagged_flex(html_text, current_theme->code_kw);
  }

<TAG_CLOSE>{IDENTIFIER} {
    append_str_flex(html_text);
  }

<TAG_CLOSE>[ \t\r]+ { append_str_flex(html_text); }
<TAG_CLOSE>.         { escape_append_flex(html_text[0]); }

  /* ── DOCTYPE state ── */

<DOCTYPE>">"   {
    BEGIN(INITIAL);
    escape_append_flex('>');
  }

<DOCTYPE>{IDENTIFIER} { append_str_flex(html_text); }
<DOCTYPE>[ \t\r]+     { append_str_flex(html_text); }
<DOCTYPE>.             { escape_append_flex(html_text[0]); }

%%

void html_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = html__scan_string(line);
  BEGIN(INITIAL);
  html_lex();
  html__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
