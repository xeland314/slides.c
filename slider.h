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
 */
void slider_render(Slider *s, int index, cairo_t *cr, int win_w, int win_h);

#endif // SLIDER_H
