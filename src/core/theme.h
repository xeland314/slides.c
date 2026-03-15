#ifndef THEME_H
#define THEME_H

typedef struct {
    const char *name;
    // Fondo
    double bg_r, bg_g, bg_b;
    // Texto
    double title_r, title_g, title_b;
    double sub_r,   sub_g,   sub_b;
    double body_r,  body_g,  body_b;
    // Elementos
    double bullet_r, bullet_g, bullet_b;
    double accent_r, accent_g, accent_b;
    double num_r,    num_g,    num_b;
    // Tablas
    double table_hdr_r, table_hdr_g, table_hdr_b;
    double table_row_r, table_row_g, table_row_b;
    double table_alt_r, table_alt_g, table_alt_b;
    double table_bdr_r, table_bdr_g, table_bdr_b;
    // Code Blocks
    double code_bg_r,   code_bg_g,   code_bg_b;
    double code_txt_r,  code_txt_g,  code_txt_b;   // Default text
    double code_kw_r,   code_kw_g,   code_kw_b;    // Keywords
    double code_com_r,  code_com_g,  code_com_b;   // Comments
    double code_str_r,  code_str_g,  code_str_b;   // Strings
    double code_sym_r,  code_sym_g,  code_sym_b;   // Symbols {} [] ()
} Theme;

// Obtener tema por nombre
const Theme* theme_find(const char *name);
// Tema por defecto
const Theme* theme_default(void);

#endif
