#include "theme.h"
#include <string.h>
#include <stddef.h>

// ── Dark (Original) ──────────────────────────────────────────────────────────
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

// ── Rose (Vino/Rosa) ─────────────────────────────────────────────────────────
static const Theme THEME_ROSE = {
    .name = "rose",
    .bg_r = 0.15, .bg_g = 0.05, .bg_b = 0.08,
    .title_r = 1.00, .title_g = 0.85, .title_b = 0.90,
    .sub_r   = 0.90, .sub_g   = 0.60, .sub_b   = 0.70,
    .body_r  = 0.95, .body_g  = 0.90, .body_b  = 0.92,
    .bullet_r = 1.00, .bullet_g = 0.40, .bullet_b = 0.60,
    .accent_r = 1.00, .accent_g = 0.30, .accent_b = 0.50,
    .num_r    = 0.70, .num_g    = 0.50, .num_b    = 0.55,
    .table_hdr_r = 0.30, .table_hdr_g = 0.10, .table_hdr_b = 0.15,
    .table_row_r = 0.20, .table_row_g = 0.08, .table_row_b = 0.10,
    .table_alt_r = 0.25, .table_alt_g = 0.10, .table_alt_b = 0.12,
    .table_bdr_r = 0.40, .table_bdr_g = 0.20, .table_bdr_b = 0.25
};

// ── Monokai (Vibrante) ────────────────────────────────────────────────────────
static const Theme THEME_MONOKAI = {
    .name = "monokai",
    .bg_r = 0.15, .bg_g = 0.16, .bg_b = 0.13,
    .title_r = 0.98, .title_g = 0.15, .title_b = 0.45, // Pink
    .sub_r   = 0.65, .sub_g   = 0.89, .sub_b   = 0.18, // Green
    .body_r  = 0.97, .body_g  = 0.97, .body_b  = 0.95, // Whiteish
    .bullet_r = 0.68, .bullet_g = 0.50, .bullet_b = 1.00, // Purple
    .accent_r = 0.40, .accent_g = 0.85, .accent_b = 0.94, // Blue
    .num_r    = 0.46, .num_g    = 0.46, .num_b    = 0.43,
    .table_hdr_r = 0.25, .table_hdr_g = 0.26, .table_hdr_b = 0.23,
    .table_row_r = 0.18, .table_row_g = 0.19, .table_row_b = 0.16,
    .table_alt_r = 0.22, .table_alt_g = 0.23, .table_alt_b = 0.20,
    .table_bdr_r = 0.36, .table_bdr_g = 0.36, .table_bdr_b = 0.33
};

// ── Nord (Ártico) ─────────────────────────────────────────────────────────────
static const Theme THEME_NORD = {
    .name = "nord",
    .bg_r = 0.18, .bg_g = 0.20, .bg_b = 0.25,
    .title_r = 0.53, .title_g = 0.75, .title_b = 0.82,
    .sub_r   = 0.51, .sub_g   = 0.63, .sub_b   = 0.76,
    .body_r  = 0.93, .body_g  = 0.94, .body_b  = 0.96,
    .bullet_r = 0.56, .bullet_g = 0.73, .bullet_b = 0.54, // Aurora Green
    .accent_r = 0.75, .accent_g = 0.38, .accent_b = 0.42, // Aurora Red
    .num_r    = 0.30, .num_g    = 0.34, .num_b    = 0.42,
    .table_hdr_r = 0.23, .table_hdr_g = 0.26, .table_hdr_b = 0.32,
    .table_row_r = 0.18, .table_row_g = 0.20, .table_row_b = 0.25,
    .table_alt_r = 0.21, .table_alt_g = 0.23, .table_alt_b = 0.29,
    .table_bdr_r = 0.30, .table_bdr_g = 0.34, .table_bdr_b = 0.42
};

// ── Light (Papel) ─────────────────────────────────────────────────────────────
static const Theme THEME_LIGHT = {
    .name = "light",
    .bg_r = 0.96, .bg_g = 0.96, .bg_b = 0.96,
    .title_r = 0.13, .title_g = 0.13, .title_b = 0.13,
    .sub_r   = 0.33, .sub_g   = 0.33, .sub_b   = 0.33,
    .body_r  = 0.20, .body_g  = 0.20, .body_b  = 0.20,
    .bullet_r = 0.00, .bullet_g = 0.48, .bullet_b = 0.80,
    .accent_r = 0.00, .accent_g = 0.48, .accent_b = 0.80,
    .num_r    = 0.60, .num_g    = 0.60, .num_b    = 0.60,
    .table_hdr_r = 0.85, .table_hdr_g = 0.85, .table_hdr_b = 0.85,
    .table_row_r = 0.96, .table_row_g = 0.96, .table_row_b = 0.96,
    .table_alt_r = 0.92, .table_alt_g = 0.92, .table_alt_b = 0.92,
    .table_bdr_r = 0.80, .table_bdr_g = 0.80, .table_bdr_b = 0.80
};

static const Theme* ALL_THEMES[] = { 
    &THEME_DARK, 
    &THEME_ROSE, 
    &THEME_MONOKAI, 
    &THEME_NORD, 
    &THEME_LIGHT 
};

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
