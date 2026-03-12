#include "../../slider.h"
#include "../core/internal.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static void print_help(const char *prog) {
    printf("Uso: %s [opciones] presentacion.md\n\n", prog);
    printf("Opciones:\n");
    printf("  -p, --palette <name>    Elegir paleta (dark, rose, monokai, nord, light)\n");
    printf("  -f, --font-family <str> Definir tipografía (ej. 'Arial', 'JetBrains Mono')\n");
    printf("  -s, --font-scale <num>  Escalar tamaño de fuentes (ej. 1.2)\n");
    printf("  -h, --help              Mostrar esta ayuda\n");
}

int main(int argc, char *argv[]) {
    const char *md_path = NULL;
    const char *palette_name = NULL;
    const char *font_family = NULL;
    double font_scale = 1.0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--palette") == 0 || strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) palette_name = argv[++i];
        } else if (strcmp(argv[i], "--font-family") == 0 || strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) font_family = argv[++i];
        } else if (strcmp(argv[i], "--font-scale") == 0 || strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) font_scale = atof(argv[++i]);
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

    Display *disp = XOpenDisplay(NULL);
    if (!disp) { fputs("No se pudo abrir display\n", stderr); return 1; }

    int scr = DefaultScreen(disp);
    int sw  = DisplayWidth(disp,  scr);
    int sh  = DisplayHeight(disp, scr);

    int wx = (sw - WIN_W) / 2;
    int wy = (sh - WIN_H) / 2;
    Window win = XCreateSimpleWindow(disp, RootWindow(disp, scr), wx, wy, WIN_W, WIN_H, 0, 0, 0);

    XStoreName(disp, win, argv[1]);
    XSelectInput(disp, win, KeyPressMask | ButtonPressMask | StructureNotifyMask);
    Atom wm_delete = XInternAtom(disp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(disp, win, &wm_delete, 1);
    Atom wm_state  = XInternAtom(disp, "_NET_WM_STATE", False);
    Atom fs_atom   = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);
    XMapWindow(disp, win);

    int win_w = WIN_W, win_h = WIN_H;
    cairo_surface_t *sfc_screen = cairo_xlib_surface_create(disp, win, DefaultVisual(disp, scr), win_w, win_h);
    cairo_surface_t *sfc_back = cairo_surface_create_similar(sfc_screen, CAIRO_CONTENT_COLOR, win_w, win_h);
    cairo_t *cr = cairo_create(sfc_back);
    cairo_t *cr_flip = cairo_create(sfc_screen);

    int current = 0, running = 1, dirty = 1, fullscreen = 0;

    while (running) {
        while (XPending(disp)) {
            XEvent ev;
            XNextEvent(disp, &ev);
            if (ev.type == KeyPress) {
                KeySym ks = XLookupKeysym(&ev.xkey, 0);
                if (ks == XK_q || ks == XK_Escape) running = 0;
                else if (ks == XK_Right || ks == XK_Return || ks == XK_space || ks == XK_Next) {
                    if (current < n_slides - 1) { current++; dirty = 1; }
                } else if (ks == XK_Left || ks == XK_BackSpace || ks == XK_Prior) {
                    if (current > 0) { current--; dirty = 1; }
                } else if (ks == XK_Home) { current = 0; dirty = 1; }
                else if (ks == XK_End) { current = n_slides - 1; dirty = 1; }
                else if (ks == XK_f || ks == XK_F11) {
                    XEvent fev = {0}; fev.type = ClientMessage; fev.xclient.window = win;
                    fev.xclient.message_type = wm_state; fev.xclient.format = 32;
                    fev.xclient.data.l[0] = fullscreen ? 0 : 1; fev.xclient.data.l[1] = (long)fs_atom;
                    XSendEvent(disp, RootWindow(disp, scr), False, SubstructureNotifyMask | SubstructureRedirectMask, &fev);
                    fullscreen = !fullscreen; dirty = 1;
                }
            }
            if (ev.type == ButtonPress) {
                if (ev.xbutton.button == Button1) { if (current > 0) { current--; dirty = 1; } }
                else if (ev.xbutton.button == Button3) { if (current < n_slides - 1) { current++; dirty = 1; } }
            }
            if (ev.type == ConfigureNotify) {
                int nw = ev.xconfigure.width, nh = ev.xconfigure.height;
                if (nw != win_w || nh != win_h) {
                    win_w = nw; win_h = nh;
                    cairo_destroy(cr); cairo_destroy(cr_flip);
                    cairo_surface_destroy(sfc_back); cairo_surface_destroy(sfc_screen);
                    cairo_xlib_surface_set_size(sfc_screen, win_w, win_h);
                    sfc_screen = cairo_xlib_surface_create(disp, win, DefaultVisual(disp, scr), win_w, win_h);
                    sfc_back = cairo_surface_create_similar(sfc_screen, CAIRO_CONTENT_COLOR, win_w, win_h);
                    cr = cairo_create(sfc_back); cr_flip = cairo_create(sfc_screen);
                    dirty = 1;
                }
            }
            if (ev.type == ClientMessage) if ((Atom)ev.xclient.data.l[0] == wm_delete) running = 0;
        }
        if (dirty) {
            cairo_set_source_rgb(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
            cairo_paint(cr);
            slider_render(s, current, cr, win_w, win_h);
            cairo_set_source_surface(cr_flip, sfc_back, 0, 0);
            cairo_paint(cr_flip);
            XFlush(disp);
            dirty = 0;
        }
        usleep(8000);
    }

    cairo_destroy(cr); cairo_destroy(cr_flip);
    cairo_surface_destroy(sfc_back); cairo_surface_destroy(sfc_screen);
    XCloseDisplay(disp);
    slider_free(s);
    return 0;
}
