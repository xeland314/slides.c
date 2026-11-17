#ifndef THEMEMENU_H
#define THEMEMENU_H

#include "../../slider.h"
#include "../core/theme.h"
#include <cairo/cairo.h>

/* Menú de temas (tecla T): cuadrícula con la diapositiva actual renderizada en
 * cada tema. Si el tema con el que se abrió la presentación es personalizado
 * (colores del frontmatter o de la CLI), aparece como primera entrada para
 * poder volver a él. Independiente de la plataforma (X11 y Win32). */

#define THEMEMENU_COLS 4

typedef struct {
    int   open;         /* 1 = menú visible */
    int   sel;          /* entrada seleccionada */
    Theme base;         /* tema con el que se abrió (incluye overrides) */
    int   base_custom;  /* 1 si base difiere de su tema integrado */
} ThemeMenu;

/* Guarda el tema actual de s como tema base */
void         thememenu_init(ThemeMenu *m, const Slider *s);
int          thememenu_count(const ThemeMenu *m);
const Theme *thememenu_theme(const ThemeMenu *m, int i);
/* Abre el menú preseleccionando el tema actual de s */
void         thememenu_open(ThemeMenu *m, Slider *s);
/* Aplica la entrada i a s y cierra el menú */
void         thememenu_apply(ThemeMenu *m, Slider *s, int i);
/* Mueve la selección (dx columnas, dy filas). Devuelve 1 si cambió */
int          thememenu_move(ThemeMenu *m, int dx, int dy);
/* Dibuja la cuadrícula con la diapositiva `current` de s en cada tema */
void         thememenu_render(const ThemeMenu *m, Slider *s, int current,
                              cairo_t *cr, int w, int h);
/* Índice de la entrada bajo (x, y) o -1 */
int          thememenu_hit(const ThemeMenu *m, int w, int h, int x, int y);

#endif
