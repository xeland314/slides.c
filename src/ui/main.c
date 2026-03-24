#include "../../slider.h"
#include "../core/internal.h"
#include "../core/theme.h"
#include "backend.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static void print_help(const char *prog) {
    printf("Uso: %s [opciones] presentacion.md\n\n", prog);
    printf("Opciones:\n");
    printf("  -p, --palette <name>    Elegir paleta (dark, rose, monokai, nord, light, blue, ambercat,\n");
    printf("                          dracula, gruvbox, catppuccin, tokyo-night)\n");
    printf("  -f, --font-family <str> Definir tipografía (ej. 'Arial', 'JetBrains Mono')\n");
    printf("  -s, --font-scale <num>  Escalar tamaño de fuentes (ej. 1.2)\n");
    printf("  -e, --export <type>     Exportar slides a 'pdf' o 'png'\n");
    printf("  -er, --export-res <WxH> Resolución de exportación (ej. 1920x1080, default 1080x1080)\n");
    printf("  -sl, --slide <num>      Seleccionar slide específico para exportar (0-index)\n");
    printf("  -h, --help              Mostrar esta ayuda\n");
}

int main(int argc, char *argv[]) {
    const char *md_path = NULL;
    const char *palette_name = NULL;
    const char *font_family = NULL;
    double font_scale = 1.0;
    int export_png = 0;
    int export_pdf = 0;
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
                else {
                    fprintf(stderr, "Error: Tipo de exportación desconocido '%s'. Use 'pdf' o 'png'.\n", type);
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
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("c-slides v1.0.0\n");
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
    if (palette_name) s->theme = theme_find(palette_name);
    if (font_family) strncpy(s->font_family, font_family, sizeof(s->font_family) - 1);
    if (font_scale > 0.1) s->font_scale = font_scale;

    int n_slides = slider_get_count(s);
    fprintf(stderr, "[slides] %d slide(s) cargados desde %s (tema: %s, font: %s, scale: %.1f)\n", 
            n_slides, md_path, s->theme->name, s->font_family, s->font_scale);

    if (export_png || export_pdf) {
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
        slider_free(s);
        return 0;
    }

    // Delegar al backend gráfico (X11, Win32, etc.)
    int ret = backend_run(s);
    
    slider_free(s);
    return ret;
}
