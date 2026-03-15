#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <stddef.h>
#include "theme.h"

/**
 * @brief Highlights a line of code using Pango markup based on theme colors.
 * @param line The raw input line of code.
 * @param theme The current theme for colors.
 * @param out Buffer to store the resulting Pango markup.
 * @param out_size Size of the output buffer.
 */
void highlighter_highlight(const char *line, const Theme *theme, char *out, size_t out_size);

#endif // HIGHLIGHTER_H
