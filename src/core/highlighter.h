#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <stddef.h>
#include "theme.h"

/**
 * @brief Prepares a full source document for syntax highlighting.
 * @param text The full document contents to parse.
 */
void highlighter_begin_document(const char *text);

/**
 * @brief Releases the currently cached document state.
 */
void highlighter_end_document(void);

/**
 * @brief Highlights a single line using the previously prepared document context.
 * @param line The raw input line of code.
 * @param line_index The zero-based line index within the prepared document.
 * @param theme The current theme for colors.
 * @param out Buffer to store the resulting Pango markup.
 * @param out_size Size of the output buffer.
 */
void highlighter_highlight_line(const char *line, size_t line_index,
                                const Theme *theme, char *out,
                                size_t out_size);

/**
 * @brief Highlights a line of code using Pango markup based on theme colors.
 * @param line The raw input line of code.
 * @param theme The current theme for colors.
 * @param out Buffer to store the resulting Pango markup.
 * @param out_size Size of the output buffer.
 */
void highlighter_highlight(const char *line, const Theme *theme, char *out,
                           size_t out_size);

#endif // HIGHLIGHTER_H
