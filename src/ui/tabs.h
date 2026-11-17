#ifndef TABS_H
#define TABS_H

#include "../../slider.h"
#include "thememenu.h"
#include <cairo/cairo.h>

/* Pestañas: varias presentaciones abiertas a la vez (independiente de la plataforma). */

#define MAX_TABS 16
#define TAB_BAR_H 30

typedef struct {
    Slider *s;
    int     current;             /* diapositiva actual */
    double  slide_start_time;    /* ms (reloj monotónico del backend) */
    double  zoom;
    int     overview;
    int     last_printed_slide;
    ThemeMenu tm;                /* menú de temas (tecla T) y tema base */
} Tab;

typedef struct {
    Tab tabs[MAX_TABS];
    int n;
    int active;
} TabSet;

typedef enum { TAB_HIT_NONE = 0, TAB_HIT_TAB, TAB_HIT_CLOSE, TAB_HIT_PLUS } TabHitKind;

typedef struct {
    TabHitKind kind;
    int index;   /* pestaña (TAB_HIT_TAB / TAB_HIT_CLOSE) */
} TabHit;

void tabs_init(TabSet *ts);
/* Libera todos los Slider */
void tabs_free(TabSet *ts);
/* Añade (toma posesión de s) y activa. Si el mismo archivo ya está abierto, libera s,
 * activa la existente y devuelve su índice. Devuelve -1 si no hay espacio (s se libera). */
int  tabs_add(TabSet *ts, Slider *s, double now_ms);
/* Cierra la pestaña i y libera su Slider */
void tabs_close(TabSet *ts, int i);
void tabs_next(TabSet *ts);
void tabs_prev(TabSet *ts);
/* Índice de la pestaña con esa ruta (realpath) o -1 */
int  tabs_find(const TabSet *ts, const char *path);
Tab *tabs_active(TabSet *ts);

void tabs_render_bar(const TabSet *ts, cairo_t *cr, int w, int bar_h);
TabHit tabs_hit_test(const TabSet *ts, int w, int bar_h, int x, int y);

#endif
