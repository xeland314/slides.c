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
} Theme;

// Obtener tema por nombre
const Theme* theme_find(const char *name);
// Tema por defecto
const Theme* theme_default(void);

#endif
