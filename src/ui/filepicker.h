#ifndef FILEPICKER_H
#define FILEPICKER_H

#include <cairo/cairo.h>
#include <limits.h>

/* Selector de archivos Markdown (independiente de la plataforma).
 * Solo dibuja con cairo/pango; las teclas llegan como eventos abstractos. */

typedef enum {
    FPK_UP, FPK_DOWN, FPK_PAGEUP, FPK_PAGEDOWN, FPK_HOME, FPK_END,
    FPK_ENTER, FPK_BACKSPACE, FPK_ESCAPE,
    FPK_TOGGLE_HIDDEN,   /* Ctrl+H */
    FPK_TEXT             /* texto UTF-8 tecleado (filtro) */
} FPKey;

typedef enum { FP_NONE = 0, FP_REDRAW, FP_CHOSEN, FP_CANCEL } FPResult;

typedef struct {
    char *name;      /* nombre (UTF-8) */
    int   is_dir;
} FPEntry;

typedef struct {
    char     dir[PATH_MAX];
    FPEntry *entries;     /* todas las entradas del directorio */
    int      n_entries;
    int     *visible;     /* índices en entries que pasan el filtro */
    int      n_visible;
    int      selected;    /* posición dentro de visible */
    int      scroll;      /* primera fila visible */
    int      rows;        /* filas que caben (actualizado en render) */
    int      show_hidden;
    char     filter[256];
    char     status[256];
    char     chosen[PATH_MAX];
    /* geometría de la última render (para clics) */
    int      list_y, row_h, last_w, last_h;
} FilePicker;

void        fp_init(FilePicker *fp, const char *start_dir);
void        fp_free(FilePicker *fp);
void        fp_render(FilePicker *fp, cairo_t *cr, int w, int h);
FPResult    fp_key(FilePicker *fp, FPKey key, const char *utf8_text);
FPResult    fp_click(FilePicker *fp, int x, int y, int is_double);
FPResult    fp_scroll(FilePicker *fp, int lines); /* >0 baja, <0 sube */
const char *fp_chosen_path(FilePicker *fp);
void        fp_set_status(FilePicker *fp, const char *msg);

#endif
