#ifndef INTERNAL_H
#define INTERNAL_H

#include <cairo/cairo.h>
#include "theme.h"

// ── Configuración ─────────────────────────────────────────────────────────────

#define WIN_W         1280
#define WIN_H         720
#define MARGIN_X      80.0
#define MARGIN_Y      60.0
#define MAX_SLIDES    256
#define MAX_LINES     128    // líneas por slide
#define MAX_LINE_LEN  1024
#define MAX_IMG_CACHE 64

// ── Definir COL_LABEL para el placeholder ────────────────────────────────────
#define COL_LABEL_R 0.55
#define COL_LABEL_G 0.65
#define COL_LABEL_B 0.90

// ── Tipos ─────────────────────────────────────────────────────────────────────

typedef enum {
    LINE_EMPTY,
    LINE_TITLE,       // # texto
    LINE_SUBTITLE,    // ## texto
    LINE_BODY,        // texto normal
    LINE_BULLET1,     // - texto
    LINE_BULLET2,     //   - texto (subviñeta)
    LINE_IMAGE,       // !ruta.png
    LINE_TABLE_ROW,   // | col | col | ...
    LINE_TABLE_SEP,   // | --- | --- | ... (separador de header)
    LINE_BLOCKQUOTE,  // > texto
} LineType;

typedef struct {
    LineType type;
    char     text[MAX_LINE_LEN];  // contenido sin prefijo
    // Para tablas: columnas parseadas
    char     cols[16][256];
    int      ncols;
} SlideLine;

typedef struct {
    SlideLine lines[MAX_LINES];
    int       nlines;
} Slide;

struct Slider {
    Slide slides[MAX_SLIDES];
    int   n_slides;
    const Theme *theme;
};

// Cache de imágenes para no recargar en cada frame
typedef struct {
    char             path[512];
    cairo_surface_t *surface;
} ImgCache;

#endif // INTERNAL_H
