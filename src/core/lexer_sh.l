%{
#include "../../slider.h"
#include "highlighter.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
%}

%option noyywrap
%option prefix="sh_"
%option noinput nounput

/* ── Definitions ── */
IDENTIFIER   [a-zA-Z_][a-zA-Z0-9_]*
SPECIAL_VAR  [$?#*@!-]
NUM_PARAM    [1-9][0-9]*
STRING_DQ    "\""([^"\\$\n]|\\.)*"\""
STRING_SQ    "'"([^'\\]|\\.)*"'"
CMD_SUBST    "`"[^`]*"`"

/* Shell keywords */
KEYWORD      "if"|"then"|"elif"|"else"|"fi"|"for"|"do"|"done"|"while"|"case"|"esac"|"in"|"function"|"select"|"until"|"time"|"coproc"

/* Shell builtins */
BUILTIN      "echo"|"printf"|"read"|"test"|"eval"|"trap"|"set"|"unset"|"shift"|"export"|"local"|"declare"|"typeset"|"readonly"|"source"|"alias"|"unalias"|"cd"|"pushd"|"popd"|"dirs"|"pwd"|"type"|"hash"|"help"|"builtin"|"command"|"enable"|"kill"|"wait"|"suspend"|"logout"|"exit"|"return"|"break"|"continue"|"jobs"|"fg"|"bg"|"disown"|"exec"|"ulimit"|"umask"|"getopts"|"caller"|"mapfile"|"readarray"|"declare"|"typeset"|"nameref"

/* Common commands */
COMMAND      "grep"|"sed"|"awk"|"find"|"xargs"|"cat"|"ls"|"sort"|"uniq"|"head"|"tail"|"wc"|"cut"|"tr"|"tee"|"touch"|"cp"|"mv"|"rm"|"mkdir"|"rmdir"|"chmod"|"chown"|"chgrp"|"ln"|"df"|"du"|"ps"|"kill"|"killall"|"nohup"|"nice"|"time"|"curl"|"wget"|"tar"|"zip"|"unzip"|"git"|"docker"|"apt"|"apt-get"|"npm"|"pip"|"systemctl"|"journalctl"|"ssh"|"scp"|"rsync"|"env"|"export"|"which"|"whereis"|"man"|"info"|"diff"|"patch"|"dd"|"mount"|"umount"|"fdisk"|"blkid"|"lsblk"|"blkdiscard"

%%

  /* ── Comments ── */
#[^\n]*             { append_tagged_flex(sh_text, current_theme->code_com); }

  /* ── Strings (single-quoted: no expansion) ── */
{STRING_SQ}         { append_tagged_flex(sh_text, current_theme->code_str); }

  /* ── Strings (double-quoted: variable expansion) ── */
{STRING_DQ}         { append_tagged_flex(sh_text, current_theme->code_str); }

  /* ── Command substitution backticks ── */
{CMD_SUBST}         { append_tagged_flex(sh_text, current_theme->code_sym); }

  /* ── Variable expansion ${...} ── */
"${"[^}]*"}"       { append_tagged_flex(sh_text, current_theme->code_kw); }

  /* ── Special variables ── */
"$"{SPECIAL_VAR}    { append_tagged_flex(sh_text, current_theme->code_kw); }

  /* ── Positional parameters ── */
"$"{NUM_PARAM}      { append_tagged_flex(sh_text, current_theme->code_kw); }

  /* ── Named variables $VAR ── */
"$"{IDENTIFIER}     { append_tagged_flex(sh_text, current_theme->code_kw); }

  /* ── Keywords ── */
{KEYWORD}           { append_tagged_flex(sh_text, current_theme->code_kw); }

  /* ── Builtins ── */
{BUILTIN}           { append_tagged_flex(sh_text, current_theme->code_sym); }

  /* ── Common commands ── */
{COMMAND}           { append_tagged_flex(sh_text, current_theme->code_sym); }

  /* ── Variables (bare identifiers that are known env vars) ── */
{IDENTIFIER}"="{IDENTIFIER} {
    /* VAR=value — highlight VAR as keyword */
    size_t eq = 0;
    while (eq < sh_leng && sh_text[eq] != '=') eq++;
    char *name = (char *)malloc(eq + 1);
    if (name) {
      memcpy(name, sh_text, eq);
      name[eq] = '\0';
      append_tagged_flex(name, current_theme->code_kw);
      free(name);
    }
    escape_append_flex(sh_text[eq]);
    const char *val = sh_text + eq + 1;
    append_str_flex(val);
  }

  /* ── Identifiers ── */
{IDENTIFIER}        { append_str_flex(sh_text); }

  /* ── Operators ── */
"&&"                { append_tagged_flex(sh_text, current_theme->code_sym); }
"||"                { append_tagged_flex(sh_text, current_theme->code_sym); }
"&>"                { append_tagged_flex(sh_text, current_theme->code_sym); }
"|&"                { append_tagged_flex(sh_text, current_theme->code_sym); }
">>"                { append_tagged_flex(sh_text, current_theme->code_sym); }
"<<-"               { append_tagged_flex(sh_text, current_theme->code_sym); }
"<<""<"             { append_tagged_flex(sh_text, current_theme->code_sym); }
">|"                { append_tagged_flex(sh_text, current_theme->code_sym); }
"|>"                { append_tagged_flex(sh_text, current_theme->code_sym); }
"=~"                { append_tagged_flex(sh_text, current_theme->code_sym); }
"!="                { append_tagged_flex(sh_text, current_theme->code_sym); }
"=="                { append_tagged_flex(sh_text, current_theme->code_sym); }
"|"                 { append_tagged_flex(sh_text, current_theme->code_sym); }
">"                 { append_tagged_flex(sh_text, current_theme->code_sym); }
"<"                 { append_tagged_flex(sh_text, current_theme->code_sym); }
"&"                 { append_tagged_flex(sh_text, current_theme->code_sym); }

  /* ── Numbers ── */
[0-9]+              { append_str_flex(sh_text); }

  /* ── Escape sequences ── */
"\\n"               { append_tagged_flex(sh_text, current_theme->code_str); }
"\\t"               { append_tagged_flex(sh_text, current_theme->code_str); }
"\\\\"              { append_tagged_flex(sh_text, current_theme->code_str); }
"\\$"               { append_tagged_flex(sh_text, current_theme->code_str); }
"\\\""              { append_tagged_flex(sh_text, current_theme->code_str); }

  /* ── Whitespace ── */
[ \t\r]+            { append_str_flex(sh_text); }

  /* ── Anything else ── */
.                   { escape_append_flex(sh_text[0]); }

%%

void sh_lexer_run(const char *line, const Theme *theme, char *out,
                  size_t out_size) {
  out_buf = out;
  out_max = out_size;
  out_idx = 0;
  current_theme = theme;
  out_buf[0] = '\0';

  YY_BUFFER_STATE buffer = sh__scan_string(line);
  sh_lex();
  sh__delete_buffer(buffer);

  out_buf[out_idx] = '\0';
}
