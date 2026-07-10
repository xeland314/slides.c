#include "theme.h"
#include <string.h>
#include <stddef.h>

// ── Dark (Pure / Neutral) ───────────────────────────────────────────────────
static const Theme THEME_DARK = {
    .name = "dark",
    .bg        = HEX(0x050505),  .title = HEX(0xffffff),  .sub   = HEX(0x999999),
    .body      = HEX(0xe6e6e6),  .bullet = HEX(0xffffff), .accent = HEX(0x808080),
    .num       = HEX(0x666666),
    .table_hdr = HEX(0x262626),  .table_row = HEX(0x0d0d0d),
    .table_alt = HEX(0x1a1a1a),  .table_bdr = HEX(0x4d4d4d),
    .code_bg   = HEX(0x000000),  .code_txt = HEX(0xf2f2f2),
    .code_kw   = HEX(0xffffff),  .code_com = HEX(0x666666),
    .code_str  = HEX(0xcccccc),  .code_sym = HEX(0xe6e6e6)
};

// ── Rose (Vino/Rosa) ─────────────────────────────────────────────────────────
static const Theme THEME_ROSE = {
    .name = "rose",
    .bg        = HEX(0x260d14),  .title = HEX(0xffd9e6),  .sub   = HEX(0xe699b2),
    .body      = HEX(0xf2e5eb),  .bullet = HEX(0xff6699), .accent = HEX(0xff4d80),
    .num       = HEX(0xb2808c),
    .table_hdr = HEX(0x4d1a26),  .table_row = HEX(0x331419),
    .table_alt = HEX(0x401a1e),  .table_bdr = HEX(0x663340),
    .code_bg   = HEX(0x1a080d),  .code_txt = HEX(0xf2d9e0),
    .code_kw   = HEX(0xff6699),  .code_com = HEX(0x80666b),
    .code_str  = HEX(0xffb233),  .code_sym = HEX(0xf2ccd9)
};

// ── Monokai (Vibrante) ────────────────────────────────────────────────────────
static const Theme THEME_MONOKAI = {
    .name = "monokai",
    .bg        = HEX(0x262921),  .title = HEX(0xfa2673),  .sub   = HEX(0xa6e32e),
    .body      = HEX(0xf7f7f2),  .bullet = HEX(0xad80ff), .accent = HEX(0x66d9f0),
    .num       = HEX(0x75756e),
    .table_hdr = HEX(0x40423b),  .table_row = HEX(0x2e3029),
    .table_alt = HEX(0x383b33),  .table_bdr = HEX(0x5c5c54),
    .code_bg   = HEX(0x141712),  .code_txt = HEX(0xf7f7f2),
    .code_kw   = HEX(0xfa2673),  .code_com = HEX(0x75756e),
    .code_str  = HEX(0xe6db73),  .code_sym = HEX(0xa6e32e)
};

// ── Nord (Ártico) ─────────────────────────────────────────────────────────────
static const Theme THEME_NORD = {
    .name = "nord",
    .bg        = HEX(0x2e3340),  .title = HEX(0x87bfd1),  .sub   = HEX(0x82a1c2),
    .body      = HEX(0xedeff5),  .bullet = HEX(0x8fba8a), .accent = HEX(0xbf616b),
    .num       = HEX(0x4c576b),
    .table_hdr = HEX(0x3b4352),  .table_row = HEX(0x2e3340),
    .table_alt = HEX(0x363b4a),  .table_bdr = HEX(0x4c576b),
    .code_bg   = HEX(0x262b38),  .code_txt = HEX(0xd9e0ed),
    .code_kw   = HEX(0x82a1c2),  .code_com = HEX(0x4c576b),
    .code_str  = HEX(0xa3bf8c),  .code_sym = HEX(0xedeff5)
};

// ── Light (Papel) ─────────────────────────────────────────────────────────────
static const Theme THEME_LIGHT = {
    .name = "light",
    .bg        = HEX(0xf5f5f5),  .title = HEX(0x212121),  .sub   = HEX(0x545454),
    .body      = HEX(0x333333),  .bullet = HEX(0x007acc), .accent = HEX(0x007acc),
    .num       = HEX(0x999999),
    .table_hdr = HEX(0xd9d9d9),  .table_row = HEX(0xf5f5f5),
    .table_alt = HEX(0xebebeb),  .table_bdr = HEX(0xcccccc),
    .code_bg   = HEX(0xe6e6e6),  .code_txt = HEX(0x333333),
    .code_kw   = HEX(0x007acc),  .code_com = HEX(0x808080),
    .code_str  = HEX(0x4d994d),  .code_sym = HEX(0x333333)
};

// ── Blue (Specialized Technical) ─────────────────────────────────────────────
static const Theme THEME_BLUE = {
    .name = "blue",
    .bg        = HEX(0x0f172a),  .title = HEX(0xffffff),  .sub   = HEX(0x38bdf8),
    .body      = HEX(0xf1f5f9),  .bullet = HEX(0x10b981), .accent = HEX(0x3b82f6),
    .num       = HEX(0xa855f7),
    .table_hdr = HEX(0x1a2e59),  .table_row = HEX(0x141f38),
    .table_alt = HEX(0x1f2947),  .table_bdr = HEX(0x38bdf8),
    .code_bg   = HEX(0x0a101c),  .code_txt = HEX(0xf1f5f9),
    .code_kw   = HEX(0xa855f7),  .code_com = HEX(0x66738c),
    .code_str  = HEX(0x10b981),  .code_sym = HEX(0xec4899)
};

// ── Ambercat (Warm Technical) ──────────────────────────────────────────────
static const Theme THEME_AMBERCAT = {
    .name = "ambercat",
    .bg        = HEX(0x483025),  .title = HEX(0xf9e6c6),  .sub   = HEX(0xf1a449),
    .body      = HEX(0xedc699),  .bullet = HEX(0xc66624), .accent = HEX(0xc66624),
    .num       = HEX(0xc66624),
    .table_hdr = HEX(0x594033),  .table_row = HEX(0x483025),
    .table_alt = HEX(0x52382e),  .table_bdr = HEX(0xc66624),
    .code_bg   = HEX(0x2e1f1a),  .code_txt = HEX(0xedc699),
    .code_kw   = HEX(0xf1a449),  .code_com = HEX(0x80665a),
    .code_str  = HEX(0xf9e6c6),  .code_sym = HEX(0xc66624)
};

// ── Dracula (Vibrant Vampire) ────────────────────────────────────────────────
static const Theme THEME_DRACULA = {
    .name = "dracula",
    .bg        = HEX(0x282a36),  .title = HEX(0xf8f8f2),  .sub   = HEX(0xbd93f9),
    .body      = HEX(0xf8f8f2),  .bullet = HEX(0xff79c6), .accent = HEX(0x50fa7b),
    .num       = HEX(0x8c8c8c),
    .table_hdr = HEX(0x45475e),  .table_row = HEX(0x282a36),
    .table_alt = HEX(0x333547),  .table_bdr = HEX(0x61637a),
    .code_bg   = HEX(0x212330),  .code_txt = HEX(0xf8f8f2),
    .code_kw   = HEX(0xff79c6),  .code_com = HEX(0x6272a4),
    .code_str  = HEX(0xf1fa8c),  .code_sym = HEX(0x8be9fd)
};

// ── Gruvbox (Retro Groove) ───────────────────────────────────────────────────
static const Theme THEME_GRUVBOX = {
    .name = "gruvbox",
    .bg        = HEX(0x282828),  .title = HEX(0xebdbb2),  .sub   = HEX(0xd65d0e),
    .body      = HEX(0xbdae93),  .bullet = HEX(0x98971a), .accent = HEX(0x458588),
    .num       = HEX(0x8c7a59),
    .table_hdr = HEX(0x3d3d3d),  .table_row = HEX(0x282828),
    .table_alt = HEX(0x333333),  .table_bdr = HEX(0x4f4d42),
    .code_bg   = HEX(0x1c1c1c),  .code_txt = HEX(0xebdbb2),
    .code_kw   = HEX(0xfa4a33),  .code_com = HEX(0x928273),
    .code_str  = HEX(0xb8ba26),  .code_sym = HEX(0xd4c438)
};

// ── Catppuccin (Soothing Pastel) ─────────────────────────────────────────────
static const Theme THEME_CATPPUCCIN = {
    .name = "catppuccin",
    .bg        = HEX(0x1e1e2e),  .title = HEX(0xcdd6f4),  .sub   = HEX(0xbac2de),
    .body      = HEX(0xa6adc8),  .bullet = HEX(0xf5c2e7), .accent = HEX(0x89dceb),
    .num       = HEX(0x595c7a),
    .table_hdr = HEX(0x31334d),  .table_row = HEX(0x1e1e2e),
    .table_alt = HEX(0x26263d),  .table_bdr = HEX(0x404561),
    .code_bg   = HEX(0x121221),  .code_txt = HEX(0xcdd6f4),
    .code_kw   = HEX(0xccb5fa),  .code_com = HEX(0x6c7087),
    .code_str  = HEX(0xa6e3a1),  .code_sym = HEX(0xfab387)
};

// ── Tokyo Night (Cyberpunk Night) ────────────────────────────────────────────
static const Theme THEME_TOKYO_NIGHT = {
    .name = "tokyo-night",
    .bg        = HEX(0x1a1b26),  .title = HEX(0xa9b1d6),  .sub   = HEX(0x787c99),
    .body      = HEX(0xc0caf5),  .bullet = HEX(0xff007c), .accent = HEX(0x7dcfff),
    .num       = HEX(0x575c80),
    .table_hdr = HEX(0x242635),  .table_row = HEX(0x1a1b26),
    .table_alt = HEX(0x1e212e),  .table_bdr = HEX(0x383d57),
    .code_bg   = HEX(0x10121c),  .code_txt = HEX(0xc0caf5),
    .code_kw   = HEX(0xbb9af7),  .code_com = HEX(0x575c7a),
    .code_str  = HEX(0x9ece6a),  .code_sym = HEX(0xff9e64)
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
