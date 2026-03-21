#ifndef SLIDER_H
#define SLIDER_H

#include <cairo/cairo.h>

/**
 * @struct Slider
 * @brief Opaque structure representing a collection of slides.
 */
typedef struct Slider Slider;

/**
 * @brief Loads slides from a markdown file.
 * @param path Path to the markdown file.
 * @return A pointer to a Slider object, or NULL on error.
 */
Slider* slider_load(const char *path);

/**
 * @brief Frees the memory associated with a Slider object.
 * @param s The Slider object to free.
 */
void slider_free(Slider *s);

/**
 * @brief Gets the number of slides in the Slider object.
 * @param s The Slider object.
 * @return The number of slides.
 */
int slider_get_count(Slider *s);

/**
 * @brief Renders a specific slide to a Cairo context.
 * @param s The Slider object.
 * @param index The index of the slide to render (0-based).
 * @param cr The Cairo context to render into.
 * @param win_w The width of the target area.
 * @param win_h The height of the target area.
 * @param time_ms Current time in milliseconds (for animations).
 */
void slider_render(Slider *s, int index, cairo_t *cr, int win_w, int win_h, double time_ms);

/**
 * @brief Exports a specific slide to a PNG file.
 * @param s The Slider object.
 * @param index The index of the slide to export (0-based).
 * @param path The output path (e.g., "slide_1.png").
 * @param w Width of the output image.
 * @param h Height of the output image.
 * @return 0 on success, non-zero on error.
 */
int slider_export_png(Slider *s, int index, const char *path, int w, int h);

/**
 * @brief Exports all slides to a single PDF file.
 * @param s The Slider object.
 * @param path The output path (e.g., "presentation.pdf").
 * @param w Width of the output pages.
 * @param h Height of the output pages.
 * @return 0 on success, non-zero on error.
 */
int slider_export_pdf(Slider *s, const char *path, int w, int h);

// ── Property accessors ───────────────────────────────────────────────────────

/**
 * @brief Sets the visual theme for the slides.
 * @param s The Slider object.
 * @param theme_name Name of the theme (e.g., "dark", "rose", "monokai", "nord", "light").
 */
void slider_set_theme(Slider *s, const char *theme_name);

/**
 * @brief Sets the font family used for rendering text.
 * @param s The Slider object.
 * @param font_family Pango-compatible font family name (e.g., "Arial", "Inter").
 */
void slider_set_font_family(Slider *s, const char *font_family);

/**
 * @brief Sets the global font scale factor.
 * @param s The Slider object.
 * @param font_scale Scale factor (1.0 is default, > 0.1).
 */
void slider_set_font_scale(Slider *s, double font_scale);

/**
 * @brief Gets the name of the currently active theme.
 * @param s The Slider object.
 * @return The theme name string.
 */
const char* slider_get_theme_name(Slider *s);

/**
 * @brief Gets the current font family name.
 * @param s The Slider object.
 * @return The font family string.
 */
const char* slider_get_font_family(Slider *s);

/**
 * @brief Gets the current font scale factor.
 * @param s The Slider object.
 * @return The font scale factor.
 */
double slider_get_font_scale(Slider *s);

#endif // SLIDER_H
