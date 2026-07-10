#ifndef THEME_H
#define THEME_H

#include <stdint.h>

#define HEX(h) ((uint32_t)(h))
#define COLOR_R(c) ((((c) >> 16) & 0xFF) / 255.0)
#define COLOR_G(c) ((((c) >> 8)  & 0xFF) / 255.0)
#define COLOR_B(c) ( ((c)        & 0xFF) / 255.0)

typedef struct {
    const char *name;
    uint32_t bg;
    uint32_t title;
    uint32_t sub;
    uint32_t body;
    uint32_t bullet;
    uint32_t accent;
    uint32_t num;
    uint32_t table_hdr;
    uint32_t table_row;
    uint32_t table_alt;
    uint32_t table_bdr;
    uint32_t code_bg;
    uint32_t code_txt;
    uint32_t code_kw;
    uint32_t code_com;
    uint32_t code_str;
    uint32_t code_sym;
} Theme;

const Theme* theme_find(const char *name);
const Theme* theme_default(void);

#endif
