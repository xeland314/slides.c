#ifndef LEXER_H
#define LEXER_H

#include "theme.h"
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

#endif // LEXER_H
