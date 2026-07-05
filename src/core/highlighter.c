#include "highlighter.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

/*
 * Enlace estático: la gramática de tree-sitter-c se compila junto con este
 * binario (parser.c vendorizado) y se enlaza directamente, sin dlopen/dlsym
 * ni dependencia de .so en tiempo de ejecución.
 *
 * Requisito de build: compilar y enlazar libtree-sitter-c (parser.c, y
 * scanner.c si la gramática lo trae) además de libtree-sitter en sí, ambas
 * de forma estática (.a) o directamente como objetos en el link final.
 */
extern const TSLanguage *tree_sitter_c(void);

typedef struct {
  TSParser *parser;
  TSTree *tree;
  const TSLanguage *language;
  char *document_text;
  size_t document_len;
} HighlighterState;

static HighlighterState g_highlighter_state;

static const char *keywords[] = {
    "if",       "else",      "for",      "while",    "do",      "switch",
    "case",     "default",   "break",    "continue", "return",  "int",
    "char",     "float",     "double",   "void",     "struct",  "typedef",
    "enum",     "static",    "extern",   "const",    "inline",  "restrict",
    "volatile", "auto",      "register", "sizeof",   "class",   "public",
    "private",  "protected", "import",   "package",  "include", "define",
    "var",      "let",       "func",     "function", "using",   "namespace",
    "try",      "catch",     "throw",    "new",      "delete",  "true",
    "false",    NULL};

static int is_keyword(const char *word) {
  for (int i = 0; keywords[i] != NULL; i++) {
    if (strcmp(word, keywords[i]) == 0)
      return 1;
  }
  return 0;
}

static void escape_append(char *out, size_t *wi, size_t out_size, char c) {
  const char *esc = NULL;
  if (c == '&')
    esc = "&amp;";
  else if (c == '<')
    esc = "&lt;";
  else if (c == '>')
    esc = "&gt;";

  if (esc) {
    size_t l = strlen(esc);
    if (*wi + l < out_size - 1) {
      memcpy(out + *wi, esc, l);
      *wi += l;
    }
  } else {
    if (*wi < out_size - 1) {
      out[*wi] = c;
      (*wi)++;
    }
  }
}

static void append_str(char *out, size_t *wi, size_t out_size, const char *s) {
  size_t l = strlen(s);
  if (*wi + l < out_size - 1) {
    memcpy(out + *wi, s, l);
    *wi += l;
  }
}

static void append_color_tag(char *out, size_t *wi, size_t out_size, double r,
                             double g, double b) {
  char buf[64];
  snprintf(buf, sizeof(buf), "<span foreground='#%02x%02x%02x'>",
           (int)(r * 255), (int)(g * 255), (int)(b * 255));
  append_str(out, wi, out_size, buf);
}

static void append_plain_text_range(char *out, size_t *wi, size_t out_size,
                                    const char *text, size_t start,
                                    size_t end) {
  for (size_t i = start; i < end; i++) {
    escape_append(out, wi, out_size, text[i]);
  }
}

static void append_span_text_range(char *out, size_t *wi, size_t out_size,
                                   const char *text, size_t start, size_t end,
                                   double r, double g, double b) {
  append_color_tag(out, wi, out_size, r, g, b);
  append_plain_text_range(out, wi, out_size, text, start, end);
  append_str(out, wi, out_size, "</span>");
}

/*
 * NOTA sobre nodos anónimos: en la gramática de tree-sitter-c, los tokens
 * literales como "if", "for", "return", "struct", etc. son nodos ANONIMOS.
 * Para ellos, ts_node_type() devuelve el propio texto del token ("if",
 * "return"...), no la cadena "keyword". Por eso, para clasificarlos
 * correctamente hace falta distinguir is_named y comparar contra la lista
 * `keywords[]` cuando el nodo no es nombrado.
 */
static int node_type_is_comment(int is_named, const char *type) {
  if (!is_named) {
    return 0;
  }
  return type != NULL && (strcmp(type, "comment") == 0 || strstr(type, "comment") != NULL);
}

static int node_type_is_string(int is_named, const char *type) {
  if (!is_named) {
    return 0;
  }
  return type != NULL && (strcmp(type, "string_literal") == 0 ||
                          strcmp(type, "string") == 0 ||
                          strcmp(type, "template_string") == 0 ||
                          strstr(type, "string") != NULL);
}

static int node_type_is_keyword(int is_named, const char *type) {
  if (type == NULL) {
    return 0;
  }
  if (!is_named) {
    /* Tokens anónimos: el "type" ES el texto literal del token. */
    return is_keyword(type);
  }
  return strcmp(type, "primitive_type") == 0 ||
         strcmp(type, "type_identifier") == 0;
}

static void compute_line_byte_range(const char *text, size_t line_index,
                                    size_t *start_byte, size_t *end_byte) {
  size_t pos = 0;
  size_t current_line = 0;

  while (text[pos] != '\0' && current_line < line_index) {
    if (text[pos] == '\n') {
      current_line++;
    }
    pos++;
  }

  *start_byte = pos;
  while (text[pos] != '\0' && text[pos] != '\n') {
    pos++;
  }
  *end_byte = pos;
}

typedef struct {
  size_t start;
  size_t end;
  const char *type;
  int is_named;
} HighlightSpan;

/*
 * Lista de spans con crecimiento dinámico. Sustituye el arreglo fijo de 64
 * elementos: una línea larga (código minificado, líneas generadas, JSON en
 * una sola línea, etc.) podía superar ese límite y truncar el resaltado en
 * silencio. Ahora crece por duplicación mediante realloc().
 */
typedef struct {
  HighlightSpan *items;
  int count;
  int capacity;
} SpanList;

static void span_list_init(SpanList *list) {
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
}

static void span_list_free(SpanList *list) {
  free(list->items);
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
}

static int span_list_push(SpanList *list, size_t start, size_t end,
                          const char *type, int is_named) {
  if (list->count >= list->capacity) {
    int new_capacity = list->capacity == 0 ? 64 : list->capacity * 2;
    HighlightSpan *new_items =
        realloc(list->items, (size_t)new_capacity * sizeof(HighlightSpan));
    if (new_items == NULL) {
      /* Sin memoria: preferimos perder este span puntual antes que
         corromper el resto del resaltado. */
      return 0;
    }
    list->items = new_items;
    list->capacity = new_capacity;
  }

  list->items[list->count].start = start;
  list->items[list->count].end = end;
  list->items[list->count].type = type;
  list->items[list->count].is_named = is_named;
  list->count++;
  return 1;
}

static void collect_leaf_spans_for_range(TSNode node, size_t line_start,
                                         size_t line_end, SpanList *spans) {
  uint32_t start_byte = ts_node_start_byte(node);
  uint32_t end_byte = ts_node_end_byte(node);

  if (end_byte <= line_start || start_byte >= line_end) {
    return;
  }

  if (ts_node_child_count(node) == 0 && start_byte < end_byte) {
    span_list_push(spans, start_byte, end_byte, ts_node_type(node),
                   ts_node_is_named(node));
    return;
  }

  for (uint32_t i = 0; i < ts_node_child_count(node); i++) {
    TSNode child = ts_node_child(node, i);
    collect_leaf_spans_for_range(child, line_start, line_end, spans);
  }
}

static int load_tree_sitter_language(void) {
  if (g_highlighter_state.language != NULL) {
    return 1;
  }

  g_highlighter_state.language = tree_sitter_c();
  return g_highlighter_state.language != NULL;
}

static void free_highlighter_state(void) {
  if (g_highlighter_state.tree != NULL) {
    ts_tree_delete(g_highlighter_state.tree);
    g_highlighter_state.tree = NULL;
  }
  if (g_highlighter_state.parser != NULL) {
    ts_parser_delete(g_highlighter_state.parser);
    g_highlighter_state.parser = NULL;
  }
  /* g_highlighter_state.language apunta a un objeto estático provisto por
     tree_sitter_c(); no se libera aquí. */
  g_highlighter_state.language = NULL;
  free(g_highlighter_state.document_text);
  g_highlighter_state.document_text = NULL;
  g_highlighter_state.document_len = 0;
}

static void highlight_fallback(const char *line, const Theme *theme, char *out,
                               size_t out_size) {
  size_t wi = 0;
  const char *p = line;

  while (*p) {
    if (p[0] == '/' && p[1] == '/') {
      append_color_tag(out, &wi, out_size, theme->code_com_r, theme->code_com_g,
                       theme->code_com_b);
      while (*p) {
        escape_append(out, &wi, out_size, *p);
        p++;
      }
      append_str(out, &wi, out_size, "</span>");
      break;
    }

    if (*p == '"' || *p == '\'') {
      char quote = *p;
      append_color_tag(out, &wi, out_size, theme->code_str_r, theme->code_str_g,
                       theme->code_str_b);
      escape_append(out, &wi, out_size, *p++);
      while (*p && *p != quote) {
        if (*p == '\\' && *(p + 1)) {
          escape_append(out, &wi, out_size, *p++);
        }
        escape_append(out, &wi, out_size, *p++);
      }
      if (*p == quote) {
        escape_append(out, &wi, out_size, *p++);
      }
      append_str(out, &wi, out_size, "</span>");
      continue;
    }

    if (isalpha((unsigned char)*p) || *p == '_' || *p == '#') {
      char word[128];
      int i = 0;
      while (*p && (isalnum((unsigned char)*p) || *p == '_' || *p == '#') && i < 127) {
        word[i++] = *p++;
      }
      word[i] = '\0';

      if (is_keyword(word) || word[0] == '#') {
        append_color_tag(out, &wi, out_size, theme->code_kw_r, theme->code_kw_g,
                         theme->code_kw_b);
        append_str(out, &wi, out_size, word);
        append_str(out, &wi, out_size, "</span>");
      } else {
        for (int j = 0; j < i; j++) {
          escape_append(out, &wi, out_size, word[j]);
        }
      }
      continue;
    }

    if (strchr("{}[]()", *p)) {
      append_color_tag(out, &wi, out_size, theme->code_sym_r, theme->code_sym_g,
                       theme->code_sym_b);
      escape_append(out, &wi, out_size, *p++);
      append_str(out, &wi, out_size, "</span>");
      continue;
    }

    escape_append(out, &wi, out_size, *p++);
  }
  out[wi] = '\0';
}

void highlighter_begin_document(const char *text) {
  if (g_highlighter_state.parser == NULL) {
    g_highlighter_state.parser = ts_parser_new();
  }

  if (g_highlighter_state.parser != NULL && load_tree_sitter_language()) {
    ts_parser_set_language(g_highlighter_state.parser, g_highlighter_state.language);
  }

  if (g_highlighter_state.document_text != NULL) {
    free(g_highlighter_state.document_text);
    g_highlighter_state.document_text = NULL;
    g_highlighter_state.document_len = 0;
  }

  if (text != NULL) {
    g_highlighter_state.document_len = strlen(text);
    g_highlighter_state.document_text = malloc(g_highlighter_state.document_len + 1);
    if (g_highlighter_state.document_text != NULL) {
      memcpy(g_highlighter_state.document_text, text, g_highlighter_state.document_len + 1);
    }
  }

  if (g_highlighter_state.parser != NULL && g_highlighter_state.language != NULL &&
      g_highlighter_state.document_text != NULL) {
    TSTree *new_tree = ts_parser_parse_string(g_highlighter_state.parser,
                                              g_highlighter_state.tree,
                                              g_highlighter_state.document_text,
                                              (uint32_t)g_highlighter_state.document_len);
    if (g_highlighter_state.tree != NULL) {
      ts_tree_delete(g_highlighter_state.tree);
    }
    g_highlighter_state.tree = new_tree;
  }
}

void highlighter_end_document(void) {
  free_highlighter_state();
}

void highlighter_highlight_line(const char *line, size_t line_index,
                                const Theme *theme, char *out,
                                size_t out_size) {
  if (out == NULL || out_size == 0) {
    return;
  }

  out[0] = '\0';
  if (line == NULL) {
    return;
  }

  if (g_highlighter_state.tree != NULL && g_highlighter_state.document_text != NULL &&
      g_highlighter_state.language != NULL) {
    size_t line_start = 0;
    size_t line_end = 0;
    compute_line_byte_range(g_highlighter_state.document_text, line_index,
                            &line_start, &line_end);

    if (line_end > line_start) {
      SpanList spans;
      span_list_init(&spans);
      collect_leaf_spans_for_range(ts_tree_root_node(g_highlighter_state.tree),
                                   line_start, line_end, &spans);

      size_t wi = 0;
      size_t cursor = line_start;
      for (int i = 0; i < spans.count; i++) {
        HighlightSpan *span = &spans.items[i];
        if (span->start > cursor) {
          append_plain_text_range(out, &wi, out_size,
                                  g_highlighter_state.document_text,
                                  cursor, span->start);
        }
        if (node_type_is_comment(span->is_named, span->type)) {
          append_span_text_range(out, &wi, out_size,
                                 g_highlighter_state.document_text,
                                 span->start, span->end,
                                 theme->code_com_r, theme->code_com_g,
                                 theme->code_com_b);
        } else if (node_type_is_string(span->is_named, span->type)) {
          append_span_text_range(out, &wi, out_size,
                                 g_highlighter_state.document_text,
                                 span->start, span->end,
                                 theme->code_str_r, theme->code_str_g,
                                 theme->code_str_b);
        } else if (node_type_is_keyword(span->is_named, span->type)) {
          append_span_text_range(out, &wi, out_size,
                                 g_highlighter_state.document_text,
                                 span->start, span->end,
                                 theme->code_kw_r, theme->code_kw_g,
                                 theme->code_kw_b);
        } else {
          append_plain_text_range(out, &wi, out_size,
                                  g_highlighter_state.document_text,
                                  span->start, span->end);
        }
        cursor = span->end;
      }
      if (cursor < line_end) {
        append_plain_text_range(out, &wi, out_size,
                                g_highlighter_state.document_text, cursor,
                                line_end);
      }
      out[wi] = '\0';
      span_list_free(&spans);
      return;
    }
  }

  highlight_fallback(line, theme, out, out_size);
}

void highlighter_highlight(const char *line, const Theme *theme, char *out,
                           size_t out_size) {
  highlighter_highlight_line(line, 0, theme, out, out_size);
}