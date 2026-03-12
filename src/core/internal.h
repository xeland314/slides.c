#ifndef INTERNAL_H
#define INTERNAL_H

#include <cairo/cairo.h>

// ── Configuración ─────────────────────────────────────────────────────────────

#define WIN_W         1280
#define WIN_H         720
#define MARGIN_X      80.0
#define MARGIN_Y      60.0
#define MAX_SLIDES    256
#define MAX_LINES     128    // líneas por slide
#define MAX_LINE_LEN  1024
#define MAX_IMG_CACHE 64

// Paleta — dark blue theme coherente con los otros programas
#define COL_BG_R      0.04
#define COL_BG_G      0.06
#define COL_BG_B      0.18

#define COL_TITLE_R   1.00
#define COL_TITLE_G   1.00
#define COL_TITLE_B   1.00

#define COL_SUB_R     0.65
#define COL_SUB_G     0.78
#define COL_SUB_B     1.00

#define COL_BODY_R    0.88
#define COL_BODY_G    0.90
#define COL_BODY_B    0.96

#define COL_BULLET_R  0.98
#define COL_BULLET_G  0.75
#define COL_BULLET_B  0.20

#define COL_TABLE_HDR_R  0.10
#define COL_TABLE_HDR_G  0.18
#define COL_TABLE_HDR_B  0.45

#define COL_TABLE_ROW_R  0.06
#define COL_TABLE_ROW_G  0.09
#define COL_TABLE_ROW_B  0.25

#define COL_TABLE_ALT_R  0.08
#define COL_TABLE_ALT_G  0.12
#define COL_TABLE_ALT_B  0.32

#define COL_TABLE_BDR_R  0.20
#define COL_TABLE_BDR_G  0.28
#define COL_TABLE_BDR_B  0.55

#define COL_ACCENT_R  0.25
#define COL_ACCENT_G  0.55
#define COL_ACCENT_B  1.00

#define COL_NUM_R     0.40
#define COL_NUM_G     0.50
#define COL_NUM_B     0.75

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
};

// Cache de imágenes para no recargar en cada frame
typedef struct {
    char             path[512];
    cairo_surface_t *surface;
} ImgCache;

#endif // INTERNAL_H
