#include "theme.h"
#include <string.h>
#include <stddef.h>

// ── Dark (Pure / Neutral) ───────────────────────────────────────────────────
static const Theme THEME_DARK = {
    .name = "dark",
    .bg_r = 0.02, .bg_g = 0.02, .bg_b = 0.02,
    .title_r = 1.00, .title_g = 1.00, .title_b = 1.00,
    .sub_r   = 0.60, .sub_g   = 0.60, .sub_b   = 0.60,
    .body_r  = 0.90, .body_g  = 0.90, .body_b  = 0.90,
    .bullet_r = 1.00, .bullet_g = 1.00, .bullet_b = 1.00,
    .accent_r = 0.50, .accent_g = 0.50, .accent_b = 0.50,
    .num_r    = 0.40, .num_g    = 0.40, .num_b    = 0.40,
    .table_hdr_r = 0.15, .table_hdr_g = 0.15, .table_hdr_b = 0.15,
    .table_row_r = 0.05, .table_row_g = 0.05, .table_row_b = 0.05,
    .table_alt_r = 0.10, .table_alt_g = 0.10, .table_alt_b = 0.10,
    .table_bdr_r = 0.30, .table_bdr_g = 0.30, .table_bdr_b = 0.30,
    // Code
    .code_bg_r = 0.00, .code_bg_g = 0.00, .code_bg_b = 0.00,
    .code_txt_r = 0.95, .code_txt_g = 0.95, .code_txt_b = 0.95,
    .code_kw_r = 1.00, .code_kw_g = 1.00, .code_kw_b = 1.00,
    .code_com_r = 0.40, .code_com_g = 0.40, .code_com_b = 0.40,
    .code_str_r = 0.80, .code_str_g = 0.80, .code_str_b = 0.80,
    .code_sym_r = 0.90, .code_sym_g = 0.90, .code_sym_b = 0.90
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
    .table_bdr_r = 0.40, .table_bdr_g = 0.20, .table_bdr_b = 0.25,
    // Code
    .code_bg_r = 0.10, .code_bg_g = 0.03, .code_bg_b = 0.05,
    .code_txt_r = 0.95, .code_txt_g = 0.85, .code_txt_b = 0.88,
    .code_kw_r = 1.00, .code_kw_g = 0.40, .code_kw_b = 0.60,
    .code_com_r = 0.50, .code_com_g = 0.40, .code_com_b = 0.42,
    .code_str_r = 1.00, .code_str_g = 0.70, .code_str_b = 0.20,
    .code_sym_r = 0.95, .code_sym_g = 0.80, .code_sym_b = 0.85
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
    .table_bdr_r = 0.36, .table_bdr_g = 0.36, .table_bdr_b = 0.33,
    // Code
    .code_bg_r = 0.08, .code_bg_g = 0.09, .code_bg_b = 0.07,
    .code_txt_r = 0.97, .code_txt_g = 0.97, .code_txt_b = 0.95,
    .code_kw_r = 0.98, .code_kw_g = 0.15, .code_kw_b = 0.45,
    .code_com_r = 0.46, .code_com_g = 0.46, .code_com_b = 0.43,
    .code_str_r = 0.90, .code_str_g = 0.86, .code_str_b = 0.45,
    .code_sym_r = 0.65, .code_sym_g = 0.89, .code_sym_b = 0.18
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
    .table_bdr_r = 0.30, .table_bdr_g = 0.34, .table_bdr_b = 0.42,
    // Code
    .code_bg_r = 0.15, .code_bg_g = 0.17, .code_bg_b = 0.22,
    .code_txt_r = 0.85, .code_txt_g = 0.88, .code_txt_b = 0.93,
    .code_kw_r = 0.51, .code_kw_g = 0.63, .code_kw_b = 0.76,
    .code_com_r = 0.30, .code_com_g = 0.34, .code_com_b = 0.42,
    .code_str_r = 0.64, .code_str_g = 0.75, .code_str_b = 0.55,
    .code_sym_r = 0.93, .code_sym_g = 0.94, .code_sym_b = 0.96
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
    .table_bdr_r = 0.80, .table_bdr_g = 0.80, .table_bdr_b = 0.80,
    // Code
    .code_bg_r = 0.90, .code_bg_g = 0.90, .code_bg_b = 0.90,
    .code_txt_r = 0.20, .code_txt_g = 0.20, .code_txt_b = 0.20,
    .code_kw_r = 0.00, .code_kw_g = 0.48, .code_kw_b = 0.80,
    .code_com_r = 0.50, .code_com_g = 0.50, .code_com_b = 0.50,
    .code_str_r = 0.30, .code_str_g = 0.60, .code_str_b = 0.30,
    .code_sym_r = 0.20, .code_sym_g = 0.20, .code_sym_b = 0.20
};

// ── Blue (Specialized Technical) ─────────────────────────────────────────────
static const Theme THEME_BLUE = {
    .name = "blue",
    .bg_r = 0.06, .bg_g = 0.09, .bg_b = 0.16, // #0f172a (Deep Slate)
    .title_r = 1.00, .title_g = 1.00, .title_b = 1.00, // #ffffff
    .sub_r   = 0.22, .sub_g   = 0.74, .sub_b   = 0.97, // #38bdf8 (Sky)
    .body_r  = 0.95, .body_g  = 0.96, .body_b  = 0.98, // #f1f5f9
    .bullet_r = 0.06, .bullet_g = 0.73, .bullet_b = 0.51, // #10b981 (Emerald)
    .accent_r = 0.23, .accent_g = 0.51, .accent_b = 0.96, // #3b82f6 (Blue)
    .num_r    = 0.66, .num_g    = 0.33, .num_b    = 0.97, // #a855f7 (Purple)
    .table_hdr_r = 0.10, .table_hdr_g = 0.18, .table_hdr_b = 0.35,
    .table_row_r = 0.08, .table_row_g = 0.12, .table_row_b = 0.22,
    .table_alt_r = 0.12, .table_alt_g = 0.16, .table_alt_b = 0.28,
    .table_bdr_r = 0.22, .table_bdr_g = 0.74, .table_bdr_b = 0.97, // Sky border
    // Code
    .code_bg_r = 0.04, .code_bg_g = 0.06, .code_bg_b = 0.11,
    .code_txt_r = 0.95, .code_txt_g = 0.96, .code_txt_b = 0.98,
    .code_kw_r = 0.66, .code_kw_g = 0.33, .code_kw_b = 0.97, // Purple
    .code_com_r = 0.40, .code_com_g = 0.45, .code_com_b = 0.55,
    .code_str_r = 0.06, .code_str_g = 0.73, .code_str_b = 0.51, // Emerald
    .code_sym_r = 0.93, .code_sym_g = 0.45, .code_sym_b = 0.60  // Pinkish #ec4899
};

// ── Ambercat (Warm Technical) ──────────────────────────────────────────────
static const Theme THEME_AMBERCAT = {
    .name = "ambercat",
    .bg_r = 0.28, .bg_g = 0.19, .bg_b = 0.15, // #483025 (Dark)
    .title_r = 0.98, .title_g = 0.90, .title_b = 0.78, // #f9e6c6 (Light)
    .sub_r   = 0.95, .sub_g   = 0.64, .sub_b   = 0.29, // #f1a449 (Vibrant)
    .body_r  = 0.93, .body_g  = 0.78, .body_b  = 0.60, // #edc699 (Muted)
    .bullet_r = 0.78, .bullet_g = 0.40, .bullet_b = 0.14, // #c66624 (Deep)
    .accent_r = 0.78, .accent_g = 0.40, .accent_b = 0.14, // #c66624
    .num_r    = 0.78, .num_g    = 0.40, .num_b    = 0.14,
    .table_hdr_r = 0.35, .table_hdr_g = 0.25, .table_hdr_b = 0.20,
    .table_row_r = 0.28, .table_row_g = 0.19, .table_row_b = 0.15,
    .table_alt_r = 0.32, .table_alt_g = 0.22, .table_alt_b = 0.18,
    .table_bdr_r = 0.78, .table_bdr_g = 0.40, .table_bdr_b = 0.14,
    // Code
    .code_bg_r = 0.18, .code_bg_g = 0.12, .code_bg_b = 0.10,
    .code_txt_r = 0.93, .code_txt_g = 0.78, .code_txt_b = 0.60,
    .code_kw_r = 0.95, .code_kw_g = 0.64, .code_kw_b = 0.29,
    .code_com_r = 0.50, .code_com_g = 0.40, .code_com_b = 0.35,
    .code_str_r = 0.98, .code_str_g = 0.90, .code_str_b = 0.78,
    .code_sym_r = 0.78, .code_sym_g = 0.40, .code_sym_b = 0.14
};

// ── Dracula (Vibrant Vampire) ────────────────────────────────────────────────
static const Theme THEME_DRACULA = {
    .name = "dracula",
    .bg_r = 0.16, .bg_g = 0.17, .bg_b = 0.24, // #282a36
    .title_r = 0.97, .title_g = 0.97, .title_b = 0.95, // #f8f8f2
    .sub_r   = 0.74, .sub_g   = 0.58, .sub_b   = 0.95, // #bd93f9
    .body_r  = 0.97, .body_g  = 0.97, .body_b  = 0.95, // #f8f8f2
    .bullet_r = 1.00, .bullet_g = 0.48, .bullet_b = 0.73, // #ff79c6
    .accent_r = 0.31, .accent_g = 0.98, .accent_b = 0.48, // #50fa7b
    .num_r    = 0.55, .num_g    = 0.55, .num_b    = 0.55,
    .table_hdr_r = 0.27, .table_hdr_g = 0.28, .table_hdr_b = 0.37,
    .table_row_r = 0.16, .table_row_g = 0.17, .table_row_b = 0.24,
    .table_alt_r = 0.20, .table_alt_g = 0.21, .table_alt_b = 0.28,
    .table_bdr_r = 0.38, .table_bdr_g = 0.39, .table_bdr_b = 0.48,
    // Code
    .code_bg_r = 0.13, .code_bg_g = 0.14, .code_bg_b = 0.19,
    .code_txt_r = 0.97, .code_txt_g = 0.97, .code_txt_b = 0.95,
    .code_kw_r = 1.00, .code_kw_g = 0.48, .code_kw_b = 0.73, // Pink
    .code_com_r = 0.38, .code_com_g = 0.41, .code_com_b = 0.54, // Comment
    .code_str_r = 0.95, .code_str_g = 0.99, .code_str_b = 0.54, // Yellow
    .code_sym_r = 0.55, .code_sym_g = 0.90, .code_sym_b = 1.00  // Cyan
};

// ── Gruvbox (Retro Groove) ───────────────────────────────────────────────────
static const Theme THEME_GRUVBOX = {
    .name = "gruvbox",
    .bg_r = 0.16, .bg_g = 0.16, .bg_b = 0.16, // #282828
    .title_r = 0.92, .title_g = 0.86, .title_b = 0.70, // #ebdbb2
    .sub_r   = 0.84, .sub_g   = 0.36, .sub_b   = 0.05, // #d65d0e
    .body_r  = 0.74, .body_g  = 0.70, .body_b  = 0.60, // #bdae93
    .bullet_r = 0.60, .bullet_g = 0.59, .bullet_b = 0.10, // #98971a
    .accent_r = 0.27, .accent_g = 0.52, .accent_b = 0.53, // #458588
    .num_r    = 0.55, .num_g    = 0.48, .num_b    = 0.35,
    .table_hdr_r = 0.24, .table_hdr_g = 0.24, .table_hdr_b = 0.24,
    .table_row_r = 0.16, .table_row_g = 0.16, .table_row_b = 0.16,
    .table_alt_r = 0.20, .table_alt_g = 0.20, .table_alt_b = 0.20,
    .table_bdr_r = 0.31, .table_bdr_g = 0.30, .table_bdr_b = 0.26,
    // Code
    .code_bg_r = 0.11, .code_bg_g = 0.11, .code_bg_b = 0.11,
    .code_txt_r = 0.92, .code_txt_g = 0.86, .code_txt_b = 0.70,
    .code_kw_r = 0.98, .code_kw_g = 0.29, .code_kw_b = 0.20, // Red
    .code_com_r = 0.57, .code_com_g = 0.51, .code_com_b = 0.45,
    .code_str_r = 0.72, .code_str_g = 0.73, .code_str_b = 0.15, // Green
    .code_sym_r = 0.83, .code_sym_g = 0.77, .code_sym_b = 0.22  // Yellow
};

// ── Catppuccin (Soothing Pastel) ─────────────────────────────────────────────
static const Theme THEME_CATPPUCCIN = {
    .name = "catppuccin",
    .bg_r = 0.11, .bg_g = 0.11, .bg_b = 0.18, // #1e1e2e (Mocha)
    .title_r = 0.80, .title_g = 0.84, .title_b = 0.96, // #cdd6f4
    .sub_r   = 0.76, .sub_g   = 0.79, .sub_b   = 0.94, // #bac2de
    .body_r  = 0.66, .body_g  = 0.70, .body_b  = 0.83, // #a6adc8
    .bullet_r = 0.96, .bullet_g = 0.71, .bullet_b = 0.89, // #f5c2e7
    .accent_r = 0.54, .accent_g = 0.91, .accent_b = 0.95, // #89dceb
    .num_r    = 0.35, .num_g    = 0.36, .num_b    = 0.48,
    .table_hdr_r = 0.19, .table_hdr_g = 0.20, .table_hdr_b = 0.30,
    .table_row_r = 0.11, .table_row_g = 0.11, .table_row_b = 0.18,
    .table_alt_r = 0.15, .table_alt_g = 0.15, .table_alt_b = 0.24,
    .table_bdr_r = 0.25, .table_bdr_g = 0.27, .table_bdr_b = 0.38,
    // Code
    .code_bg_r = 0.07, .code_bg_g = 0.07, .code_bg_b = 0.13,
    .code_txt_r = 0.80, .code_txt_g = 0.84, .code_txt_b = 0.96,
    .code_kw_r = 0.80, .code_kw_g = 0.71, .code_kw_b = 0.98, // Mauve
    .code_com_r = 0.42, .code_com_g = 0.44, .code_com_b = 0.53,
    .code_str_r = 0.65, .code_str_g = 0.89, .code_str_b = 0.63, // Green
    .code_sym_r = 0.98, .code_sym_g = 0.80, .code_sym_b = 0.69  // Peach
};

// ── Tokyo Night (Cyberpunk Night) ────────────────────────────────────────────
static const Theme THEME_TOKYO_NIGHT = {
    .name = "tokyo-night",
    .bg_r = 0.09, .bg_g = 0.10, .bg_b = 0.14, // #1a1b26
    .title_r = 0.65, .title_g = 0.74, .title_b = 1.00, // #a9b1d6
    .sub_r   = 0.47, .sub_g   = 0.54, .sub_b   = 0.78, // #787c99
    .body_r  = 0.77, .body_g  = 0.82, .body_b  = 0.95, // #c0caf5
    .bullet_r = 0.99, .bullet_g = 0.45, .bullet_b = 0.64, // #ff007c
    .accent_r = 0.48, .accent_g = 0.89, .accent_b = 1.00, // #7dcfff
    .num_r    = 0.34, .num_g    = 0.36, .num_b    = 0.50,
    .table_hdr_r = 0.14, .table_hdr_g = 0.15, .table_hdr_b = 0.21,
    .table_row_r = 0.09, .table_row_g = 0.10, .table_row_b = 0.14,
    .table_alt_r = 0.11, .table_alt_g = 0.13, .table_alt_b = 0.18,
    .table_bdr_r = 0.22, .table_bdr_g = 0.24, .table_bdr_b = 0.34,
    // Code
    .code_bg_r = 0.06, .code_bg_g = 0.07, .code_bg_b = 0.11,
    .code_txt_r = 0.77, .code_txt_g = 0.82, .code_txt_b = 0.95,
    .code_kw_r = 0.73, .code_kw_g = 0.58, .code_kw_b = 1.00, // #bb9af7
    .code_com_r = 0.34, .code_com_g = 0.36, .code_com_b = 0.48,
    .code_str_r = 0.60, .code_str_g = 0.84, .code_str_b = 0.47, // #9ece6a
    .code_sym_r = 1.00, .code_sym_g = 0.62, .code_sym_b = 0.41  // #ff9e64
};

static const Theme* ALL_THEMES[] = { 
    &THEME_DARK, 
    &THEME_ROSE, 
    &THEME_MONOKAI, 
    &THEME_NORD, 
    &THEME_LIGHT,
    &THEME_BLUE,
    &THEME_AMBERCAT,
    &THEME_DRACULA,
    &THEME_GRUVBOX,
    &THEME_CATPPUCCIN,
    &THEME_TOKYO_NIGHT
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
