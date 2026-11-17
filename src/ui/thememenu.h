#ifndef THEMEMENU_H
#define THEMEMENU_H

#include "tabs.h"
#include <cairo/cairo.h>

/* Menú de temas (tecla T): cuadrícula con la diapositiva actual renderizada en
 * cada tema. Si el tema con el que se abrió la presentación es personalizado
 * (colores del frontmatter o de la CLI), aparece como primera entrada para
 * poder volver a él. Independiente de la plataforma. */

#define THEMEMENU_COLS 4

int          thememenu_count(const Tab *t);
const Theme *thememenu_theme(const Tab *t, int i);
/* Abre el menú preseleccionando el tema actual */
void         thememenu_open(Tab *t);
/* Aplica la entrada i al Slider de la pestaña y cierra el menú */
void         thememenu_apply(Tab *t, int i);
/* Mueve la selección (dx columnas, dy filas). Devuelve 1 si cambió */
int          thememenu_move(Tab *t, int dx, int dy);
void         thememenu_render(Tab *t, cairo_t *cr, int w, int h);
/* Índice de la entrada bajo (x, y) o -1 */
int          thememenu_hit(const Tab *t, int w, int h, int x, int y);

#endif
