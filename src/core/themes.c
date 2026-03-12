#include "theme.h"
#include <string.h>
#include <stddef.h>

// El tema original "Dark Blue"
static const Theme THEME_DARK = {
    .name = "dark",
    .bg_r = 0.04, .bg_g = 0.06, .bg_b = 0.18,
    .title_r = 1.00, .title_g = 1.00, .title_b = 1.00,
    .sub_r   = 0.65, .sub_g   = 0.78, .sub_b   = 1.00,
    .body_r  = 0.88, .body_g  = 0.90, .body_b  = 0.96,
    .bullet_r = 0.98, .bullet_g = 0.75, .bullet_b = 0.20,
    .accent_r = 0.25, .accent_g = 0.55, .accent_b = 1.00,
    .num_r    = 0.40, .num_g    = 0.50, .num_b    = 0.75,
    .table_hdr_r = 0.10, .table_hdr_g = 0.18, .table_hdr_b = 0.45,
    .table_row_r = 0.06, .table_row_g = 0.09, .table_row_b = 0.25,
    .table_alt_r = 0.08, .table_alt_g = 0.12, .table_alt_b = 0.32,
    .table_bdr_r = 0.20, .table_bdr_g = 0.28, .table_bdr_b = 0.55
};

// El tema nuevo solicitado "Rose"
static const Theme THEME_ROSE = {
    .name = "rose",
    .bg_r = 0.15, .bg_g = 0.05, .bg_b = 0.08,  // fondo vino/rosa muy oscuro
    .title_r = 1.00, .title_g = 0.85, .title_b = 0.90, // titulo rosado claro
    .sub_r   = 0.90, .sub_g   = 0.60, .sub_b   = 0.70, // subtitulo rosa medio
    .body_r  = 0.95, .body_g  = 0.90, .body_b  = 0.92, // cuerpo blanquecino rosa
    .bullet_r = 1.00, .bullet_g = 0.40, .bullet_b = 0.60, // viñetas rosa fuerte
    .accent_r = 1.00, .accent_g = 0.30, .accent_b = 0.50, // acento magenta
    .num_r    = 0.70, .num_g    = 0.50, .num_b    = 0.55,
    .table_hdr_r = 0.30, .table_hdr_g = 0.10, .table_hdr_b = 0.15,
    .table_row_r = 0.20, .table_row_g = 0.08, .table_row_b = 0.10,
    .table_alt_r = 0.25, .table_alt_g = 0.10, .table_alt_b = 0.12,
    .table_bdr_r = 0.40, .table_bdr_g = 0.20, .table_bdr_b = 0.25
};

static const Theme* ALL_THEMES[] = { &THEME_DARK, &THEME_ROSE };

const Theme* theme_find(const char *name) {
    if (!name) return &THEME_DARK;
    for (size_t i = 0; i < sizeof(ALL_THEMES)/sizeof(ALL_THEMES[0]); i++) {
        if (strcmp(ALL_THEMES[i]->name, name) == 0)
            return ALL_THEMES[i];
    }
    return &THEME_DARK;
}

const Theme* theme_default(void) {
    return &THEME_DARK;
}
