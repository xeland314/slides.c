#include "../../slider.h"
#include "../core/internal.h"
#include "../core/theme.h"
#include "backend.h"
#include "help.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *md_path = NULL;
    const char *palette_name = NULL;
    const char *font_family = NULL;
    double font_scale = -1.0;
    int kiosk_seconds = 0;
    const char *color_bg = NULL, *color_title = NULL, *color_sub = NULL, *color_text = NULL, *color_bullet = NULL;
    const char *color_accent = NULL, *color_num = NULL;
    const char *color_table_hdr = NULL, *color_table_row = NULL, *color_table_alt = NULL, *color_table_bdr = NULL;
    const char *color_code_bg = NULL, *color_code_txt = NULL, *color_code_kw = NULL, *color_code_com = NULL, *color_code_str = NULL, *color_code_sym = NULL;
    int export_png = 0;
    int export_pdf = 0;
    int export_svg = 0;
    int export_gif = 0;
    int export_jpg = 0;
    int target_slide = -1;
    int export_w = 1080;
    int export_h = 1080;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--palette") == 0 || strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) palette_name = argv[++i];
        } else if (strcmp(argv[i], "--font-family") == 0 || strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) font_family = argv[++i];
        } else if (strcmp(argv[i], "--font-scale") == 0 || strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) font_scale = atof(argv[++i]);
        } else if (strcmp(argv[i], "--export") == 0 || strcmp(argv[i], "-e") == 0) {
            if (i + 1 < argc) {
                const char *type = argv[++i];
                if (strcmp(type, "pdf") == 0) export_pdf = 1;
                else if (strcmp(type, "png") == 0) export_png = 1;
                else if (strcmp(type, "svg") == 0) export_svg = 1;
                else if (strcmp(type, "gif") == 0) export_gif = 1;
                else if (strcmp(type, "jpg") == 0 || strcmp(type, "jpeg") == 0) export_jpg = 1;
                else {
                    fprintf(stderr, "Error: Tipo de exportación desconocido '%s'. Use 'pdf', 'png', 'svg', 'gif' o 'jpg'.\n", type);
                    return 1;
                }
            } else {
                // Default a png si no se especifica tipo
                export_png = 1;
            }
        } else if (strcmp(argv[i], "--export-res") == 0 || strcmp(argv[i], "-er") == 0) {
            if (i + 1 < argc) {
                sscanf(argv[++i], "%dx%d", &export_w, &export_h);
            }
        } else if (strcmp(argv[i], "--slide") == 0 || strcmp(argv[i], "-sl") == 0) {
            if (i + 1 < argc) target_slide = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--bg") == 0) {
            if (i + 1 < argc) color_bg = argv[++i];
        } else if (strcmp(argv[i], "--title") == 0) {
            if (i + 1 < argc) color_title = argv[++i];
        } else if (strcmp(argv[i], "--sub") == 0 || strcmp(argv[i], "--subtitle") == 0) {
            if (i + 1 < argc) color_sub = argv[++i];
        } else if (strcmp(argv[i], "--text") == 0 || strcmp(argv[i], "--body") == 0) {
            if (i + 1 < argc) color_text = argv[++i];
        } else if (strcmp(argv[i], "--bullet") == 0) {
            if (i + 1 < argc) color_bullet = argv[++i];
        } else if (strcmp(argv[i], "--accent") == 0) {
            if (i + 1 < argc) color_accent = argv[++i];
        } else if (strcmp(argv[i], "--kiosk") == 0 || strcmp(argv[i], "--auto-advance") == 0) {
            if (i + 1 < argc) kiosk_seconds = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--num") == 0 || strcmp(argv[i], "--number") == 0) {
            if (i + 1 < argc) color_num = argv[++i];
        } else if (strcmp(argv[i], "--table-hdr") == 0) {
            if (i + 1 < argc) color_table_hdr = argv[++i];
        } else if (strcmp(argv[i], "--table-row") == 0) {
            if (i + 1 < argc) color_table_row = argv[++i];
        } else if (strcmp(argv[i], "--table-alt") == 0) {
            if (i + 1 < argc) color_table_alt = argv[++i];
        } else if (strcmp(argv[i], "--table-bdr") == 0) {
            if (i + 1 < argc) color_table_bdr = argv[++i];
        } else if (strcmp(argv[i], "--code-bg") == 0) {
            if (i + 1 < argc) color_code_bg = argv[++i];
        } else if (strcmp(argv[i], "--code-txt") == 0) {
            if (i + 1 < argc) color_code_txt = argv[++i];
        } else if (strcmp(argv[i], "--code-kw") == 0) {
            if (i + 1 < argc) color_code_kw = argv[++i];
        } else if (strcmp(argv[i], "--code-com") == 0) {
            if (i + 1 < argc) color_code_com = argv[++i];
        } else if (strcmp(argv[i], "--code-str") == 0) {
            if (i + 1 < argc) color_code_str = argv[++i];
        } else if (strcmp(argv[i], "--code-sym") == 0) {
            if (i + 1 < argc) color_code_sym = argv[++i];
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        } else {
            md_path = argv[i];
        }
    }

    if (!md_path) {
        print_help(argv[0]);
        return 1;
    }

    Slider *s = slider_load(md_path);
    if (!s) return 1;
    if (palette_name) slider_set_theme(s, palette_name);
    if (font_family) strncpy(s->font_family, font_family, sizeof(s->font_family) - 1);
    if (font_scale > 0.1) s->font_scale = font_scale;
    if (color_bg) slider_set_color(s, "bg", color_bg);
    if (color_title) slider_set_color(s, "title", color_title);
    if (color_sub) slider_set_color(s, "sub", color_sub);
    if (color_text) slider_set_color(s, "text", color_text);
    if (color_bullet) slider_set_color(s, "bullet", color_bullet);
    if (color_accent) slider_set_color(s, "accent", color_accent);
    if (kiosk_seconds > 0) s->kiosk_interval_ms = kiosk_seconds * 1000;
    if (color_num) slider_set_color(s, "num", color_num);
    if (color_table_hdr) slider_set_color(s, "table-hdr", color_table_hdr);
    if (color_table_row) slider_set_color(s, "table-row", color_table_row);
    if (color_table_alt) slider_set_color(s, "table-alt", color_table_alt);
    if (color_table_bdr) slider_set_color(s, "table-bdr", color_table_bdr);
    if (color_code_bg) slider_set_color(s, "code-bg", color_code_bg);
    if (color_code_txt) slider_set_color(s, "code-txt", color_code_txt);
    if (color_code_kw) slider_set_color(s, "code-kw", color_code_kw);
    if (color_code_com) slider_set_color(s, "code-com", color_code_com);
    if (color_code_str) slider_set_color(s, "code-str", color_code_str);
    if (color_code_sym) slider_set_color(s, "code-sym", color_code_sym);

    int n_slides = slider_get_count(s);
    fprintf(stderr, "[slides] %d slide(s) cargados desde %s (tema: %s, font: %s, scale: %.1f)\n", 
            n_slides, md_path, s->theme->name, s->font_family, s->font_scale);

    if (export_png || export_pdf || export_svg || export_gif || export_jpg) {
        if (export_pdf) {
            char filename[1024];
            strncpy(filename, md_path, sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';
            
            // Quitar extensión .md si existe y añadir .pdf
            char *dot = strrchr(filename, '.');
            if (dot && strcmp(dot, ".md") == 0) {
                strcpy(dot, ".pdf");
            } else {
                strncat(filename, ".pdf", sizeof(filename) - strlen(filename) - 1);
            }

            if (slider_export_pdf(s, filename, export_w, export_h) == 0) {
                printf("Exportado PDF: %s (%dx%d)\n", filename, export_w, export_h);
            } else {
                fprintf(stderr, "Fallo al exportar PDF: %s\n", filename);
            }
        }

        if (export_svg) {
            int start = (target_slide >= 0) ? target_slide : 0;
            int end   = (target_slide >= 0) ? target_slide + 1 : n_slides;
            
            if (start < 0 || start >= n_slides) {
                fprintf(stderr, "Error: Slide %d fuera de rango (0-%d)\n", target_slide, n_slides-1);
                slider_free(s);
                return 1;
            }

            for (int i = start; i < end; i++) {
                char filename[1024];
                snprintf(filename, sizeof(filename), "slide_%d.svg", i + 1);
                if (slider_export_svg(s, i, filename, export_w, export_h) == 0) {
                    printf("Exportado SVG: %s (%dx%d)\n", filename, export_w, export_h);
                } else {
                    fprintf(stderr, "Fallo al exportar SVG: %s\n", filename);
                }
            }
        }

        if (export_gif) {
            char filename[1024];
            strncpy(filename, md_path, sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';
            char *dot = strrchr(filename, '.');
            if (dot && strcmp(dot, ".md") == 0) {
                strcpy(dot, ".gif");
            } else {
                strncat(filename, ".gif", sizeof(filename) - strlen(filename) - 1);
            }

            if (slider_export_gif(s, filename, export_w, export_h) == 0) {
                printf("Exportado GIF: %s (%dx%d, %d slides animados)\n", filename, export_w, export_h, n_slides);
            } else {
                fprintf(stderr, "Fallo al exportar GIF: %s\n", filename);
            }
        }

        if (export_png) {
            int start = (target_slide >= 0) ? target_slide : 0;
            int end   = (target_slide >= 0) ? target_slide + 1 : n_slides;
            
            if (start < 0 || start >= n_slides) {
                fprintf(stderr, "Error: Slide %d fuera de rango (0-%d)\n", target_slide, n_slides-1);
                slider_free(s);
                return 1;
            }

            for (int i = start; i < end; i++) {
                char filename[1024];
                snprintf(filename, sizeof(filename), "slide_%d.png", i + 1);
                if (slider_export_png(s, i, filename, export_w, export_h) == 0) {
                    printf("Exportado PNG: %s (%dx%d)\n", filename, export_w, export_h);
                } else {
                    fprintf(stderr, "Fallo al exportar PNG: %s\n", filename);
                }
            }
        }

        if (export_jpg) {
            int start = (target_slide >= 0) ? target_slide : 0;
            int end   = (target_slide >= 0) ? target_slide + 1 : n_slides;

            if (start < 0 || start >= n_slides) {
                fprintf(stderr, "Error: Slide %d fuera de rango (0-%d)\n", target_slide, n_slides-1);
                slider_free(s);
                return 1;
            }

            for (int i = start; i < end; i++) {
                char filename[1024];
                snprintf(filename, sizeof(filename), "slide_%d.jpg", i + 1);
                if (slider_export_jpg(s, i, filename, export_w, export_h, 90) == 0) {
                    printf("Exportado JPG: %s (%dx%d)\n", filename, export_w, export_h);
                } else {
                    fprintf(stderr, "Fallo al exportar JPG: %s\n", filename);
                }
            }
        }

        slider_free(s);
        return 0;
    }

    // Delegar al backend gráfico (X11, Win32, etc.)
    int ret = backend_run(s);
    
    slider_free(s);
    return ret;
}
