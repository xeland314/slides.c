#include "../core/internal.h" // Para WIN_W, WIN_H, etc.
#include "backend.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <string.h>   // Añadido para strncpy
#include <sys/stat.h> // Añadido para struct stat y stat()
#include <time.h>
#include <unistd.h>

static double get_time_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

int backend_run(Slider *s) {
  if (!s)
    return 1;

  Display *disp = XOpenDisplay(NULL);
  if (!disp) {
    fputs("No se pudo abrir display X11\n", stderr);
    return 1;
  }

  int scr = DefaultScreen(disp);
  int sw = DisplayWidth(disp, scr);
  int sh = DisplayHeight(disp, scr);

  int wx = (sw - WIN_W) / 2;
  int wy = (sh - WIN_H) / 2;
  Window win = XCreateSimpleWindow(disp, RootWindow(disp, scr), wx, wy, WIN_W,
                                   WIN_H, 0, 0, 0);

  XStoreName(disp, win, "C-Slides");
  XSelectInput(disp, win,
               KeyPressMask | ButtonPressMask | StructureNotifyMask |
                   ExposureMask);
  Atom wm_delete = XInternAtom(disp, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(disp, win, &wm_delete, 1);
  Atom wm_state = XInternAtom(disp, "_NET_WM_STATE", False);
  Atom fs_atom = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);
  XMapWindow(disp, win);

  int win_w = WIN_W, win_h = WIN_H;
  cairo_surface_t *sfc_screen = cairo_xlib_surface_create(
      disp, win, DefaultVisual(disp, scr), win_w, win_h);
  cairo_surface_t *sfc_back = cairo_surface_create_similar(
      sfc_screen, CAIRO_CONTENT_COLOR, win_w, win_h);
  cairo_t *cr = cairo_create(sfc_back);
  cairo_t *cr_flip = cairo_create(sfc_screen);

  int current = 0, running = 1, dirty = 1, fullscreen = 0,
      last_printed_slide = -1;
  int overview_active = 0;
  double zoom = 1.0;
  int n_slides = slider_get_count(s);
#define OVERVIEW_COLS 4

  double start_time = get_time_ms();
  double slide_start_time = start_time;
  double last_check_time =
      0; // Para controlar el intervalo de recarga (hot reload)

  while (running) {
    while (XPending(disp)) {
      XEvent ev;
      XNextEvent(disp, &ev);
      if (ev.type == KeyPress) {
        KeySym ks = XLookupKeysym(&ev.xkey, 0);
        if (ks == XK_Tab) {
          overview_active = !overview_active;
          if (overview_active)
            s->transition_type = TRANS_NONE;
          dirty = 1;
        } else if (overview_active) {
          if (ks == XK_Escape) {
            overview_active = 0;
            s->transition_type = TRANS_NONE;
            dirty = 1;
          } else if (ks == XK_Right || ks == XK_Down) {
            if (current < n_slides - 1) {
              current++;
              dirty = 1;
            }
          } else if (ks == XK_Left || ks == XK_Up) {
            if (current > 0) {
              current--;
              dirty = 1;
            }
          } else if (ks == XK_Home) {
            current = 0;
            dirty = 1;
          } else if (ks == XK_End) {
            current = n_slides - 1;
            dirty = 1;
          } else if (ks == XK_Return || ks == XK_space) {
            overview_active = 0;
            s->transition_type = TRANS_NONE;
            slide_start_time = get_time_ms();
            dirty = 1;
          }
        } else if ((ks == XK_plus || ks == XK_equal || ks == XK_KP_Add) &&
                   (ev.xkey.state & ControlMask)) {
          zoom *= 1.2;
          if (zoom > 5.0)
            zoom = 5.0;
          dirty = 1;
        } else if ((ks == XK_minus || ks == XK_underscore ||
                    ks == XK_KP_Subtract) &&
                   (ev.xkey.state & ControlMask)) {
          zoom /= 1.2;
          if (zoom < 0.3)
            zoom = 0.3;
          dirty = 1;
        } else if (ks == XK_0 && (ev.xkey.state & ControlMask)) {
          zoom = 1.0;
          dirty = 1;
        } else if (ks == XK_q || ks == XK_Escape)
          running = 0;
        else if (ks == XK_Right || ks == XK_Return || ks == XK_space ||
                 ks == XK_Next) {
          if (current < n_slides - 1) {
            s->transition_from = current;
            s->transition_type = s->slides[current + 1].transition;
            current++;
            dirty = 1;
            slide_start_time = get_time_ms();
          }
        } else if (ks == XK_Left || ks == XK_BackSpace || ks == XK_Prior) {
          if (current > 0) {
            s->transition_from = current;
            s->transition_type = s->slides[current - 1].transition;
            current--;
            dirty = 1;
            slide_start_time = get_time_ms();
          }
        } else if (ks == XK_Home) {
          s->transition_from = current;
          s->transition_type = s->slides[0].transition;
          current = 0;
          dirty = 1;
          slide_start_time = get_time_ms();
        } else if (ks == XK_End) {
          s->transition_from = current;
          s->transition_type = s->slides[n_slides - 1].transition;
          current = n_slides - 1;
          dirty = 1;
          slide_start_time = get_time_ms();
        } else if (ks == XK_n || ks == XK_N) {
          s->hide_num = !s->hide_num;
          dirty = 1;
        } else if (ks == XK_f || ks == XK_F11) {
          XEvent fev = {0};
          fev.type = ClientMessage;
          fev.xclient.window = win;
          fev.xclient.message_type = wm_state;
          fev.xclient.format = 32;
          fev.xclient.data.l[0] = fullscreen ? 0 : 1;
          fev.xclient.data.l[1] = (long)fs_atom;
          XSendEvent(disp, RootWindow(disp, scr), False,
                     SubstructureNotifyMask | SubstructureRedirectMask, &fev);
          fullscreen = !fullscreen;
          dirty = 1;
        }
      }
      if (ev.type == ButtonPress) {
        if (overview_active) {
          int cols = OVERVIEW_COLS;
          double tw = (double)win_w / cols;
          double th = (double)win_h / ((n_slides + cols - 1) / cols);
          int col = ev.xbutton.x / tw;
          int row = (ev.xbutton.y - 30) / (th - 30);
          if (ev.xbutton.y < 30)
            row = -1;
          int idx = row * cols + col;
          if (row >= 0 && col >= 0 && col < cols && idx >= 0 &&
              idx < n_slides) {
            if (idx != current) {
              current = idx;
              slide_start_time = get_time_ms();
            }
            overview_active = 0;
            s->transition_type = TRANS_NONE;
            dirty = 1;
          } else {
            overview_active = 0;
            s->transition_type = TRANS_NONE;
            dirty = 1;
          }
        } else if (ev.xbutton.button == Button1 ||
                   ev.xbutton.button == Button4) {
          if (current > 0) {
            s->transition_from = current;
            s->transition_type = s->slides[current - 1].transition;
            current--;
            dirty = 1;
            slide_start_time = get_time_ms();
          }
        } else if (ev.xbutton.button == Button3 ||
                   ev.xbutton.button == Button5) {
          if (current < n_slides - 1) {
            s->transition_from = current;
            s->transition_type = s->slides[current + 1].transition;
            current++;
            dirty = 1;
            slide_start_time = get_time_ms();
          }
        }
      }
      if (ev.type == ConfigureNotify) {
        int nw = ev.xconfigure.width, nh = ev.xconfigure.height;
        if (nw != win_w || nh != win_h) {
          win_w = nw;
          win_h = nh;
          cairo_destroy(cr);
          cairo_destroy(cr_flip);
          cairo_surface_destroy(sfc_back);
          cairo_surface_destroy(sfc_screen);
          cairo_xlib_surface_set_size(sfc_screen, win_w, win_h);
          sfc_screen = cairo_xlib_surface_create(
              disp, win, DefaultVisual(disp, scr), win_w, win_h);
          sfc_back = cairo_surface_create_similar(
              sfc_screen, CAIRO_CONTENT_COLOR, win_w, win_h);
          cr = cairo_create(sfc_back);
          cr_flip = cairo_create(sfc_screen);
          dirty = 1;
        }
      }
      if (ev.type == Expose) {
        dirty = 1;
      }
      if (ev.type == ClientMessage)
        if ((Atom)ev.xclient.data.l[0] == wm_delete)
          running = 0;
    }

    double now_time = get_time_ms();
    if (now_time - last_check_time >
        500.0) { // Comprobar cada 500ms igual que Win32
      last_check_time = now_time;
      struct stat st;
      if (s && stat(s->filepath, &st) == 0) {
        if ((long long)st.st_mtime > s->last_mtime) {
          Slider *new_s = slider_load(s->filepath);
          if (new_s) {
            // Conservar propiedades pasadas por CLI
            new_s->theme_storage = s->theme_storage;
            new_s->theme = &new_s->theme_storage;
            strncpy(new_s->font_family, s->font_family,
                    sizeof(new_s->font_family) - 1);
            new_s->font_scale = s->font_scale;

            // Liberar el anterior y apuntar al nuevo puntero
            slider_free(s);
            s = new_s;
            n_slides = slider_get_count(s);
            if (current >= n_slides)
              current = n_slides - 1;

            dirty = 1;
            last_printed_slide = -1; // Forzar refresco de notas en consola
            fprintf(stderr, "[slides] Diapositivas recargadas automáticamente "
                            "(hot reload)\n");
          }
        }
      }
    }

    double elapsed = get_time_ms() - slide_start_time;
    if (s->slides[current].has_anim || elapsed < TRANSITION_DEFAULT_MS)
      dirty = 1;

    if (dirty) {
      if (!overview_active && current != last_printed_slide) {
        slider_print_notes(s, current);
        last_printed_slide = current;
      }
      double now = get_time_ms() - slide_start_time;
      cairo_set_source_rgb(cr, s->theme->bg_r, s->theme->bg_g, s->theme->bg_b);
      cairo_paint(cr);

      if (overview_active) {
        int n = n_slides;
        int cols = OVERVIEW_COLS;
        int rows = (n + cols - 1) / cols;
        double tw = (double)win_w / cols;
        double th = (double)win_h / rows;

        cairo_set_source_rgb(cr, 0.08, 0.08, 0.1);
        cairo_paint(cr);

        cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 16);
        cairo_set_source_rgb(cr, 0.6, 0.6, 0.65);
        cairo_move_to(cr, 12, 22);
        cairo_show_text(cr,
                        "OVERVIEW  [TAB toggle, ESC exit, click to select]");

        for (int i = 0; i < n; i++) {
          int col = i % cols;
          int row = i / cols;
          double cx = col * tw;
          double cy = row * th + 30;
          double cell_h = th - 30;
          if (cell_h < 20)
            cell_h = th;

          double sx = (tw - 8) / win_w;
          double sy = (cell_h - 20) / win_h;
          double scl = (sx < sy) ? sx : sy;
          double ox = cx + (tw - win_w * scl) / 2;
          double oy = cy + 4;

          cairo_save(cr);
          cairo_translate(cr, ox, oy);
          cairo_scale(cr, scl, scl);
          cairo_rectangle(cr, 0, 0, win_w, win_h);
          cairo_clip(cr);
          slider_render(s, i, cr, win_w, win_h, 0.0);
          cairo_restore(cr);

          cairo_rectangle(cr, cx + 2, cy + 2, tw - 4, cell_h - 4);
          if (i == current) {
            cairo_set_source_rgb(cr, 1, 0.84, 0);
            cairo_set_line_width(cr, 3);
          } else {
            cairo_set_source_rgb(cr, 0.3, 0.3, 0.35);
            cairo_set_line_width(cr, 1);
          }
          cairo_stroke(cr);

          char label[32];
          snprintf(label, sizeof(label), "%d / %d", i + 1, n);
          cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                                 CAIRO_FONT_WEIGHT_NORMAL);
          cairo_set_font_size(cr, 11);
          cairo_set_source_rgb(cr, 0.55, 0.55, 0.6);
          cairo_text_extents_t te;
          cairo_text_extents(cr, label, &te);
          cairo_move_to(cr, cx + (tw - te.width) / 2, cy + cell_h - 6);
          cairo_show_text(cr, label);
        }
      } else {
        cairo_save(cr);
        if (zoom != 1.0) {
          cairo_translate(cr, win_w / 2.0, win_h / 2.0);
          cairo_scale(cr, zoom, zoom);
          cairo_translate(cr, -win_w / 2.0, -win_h / 2.0);
        }
        slider_render(s, current, cr, win_w, win_h, now);
        cairo_restore(cr);
      }

      cairo_set_source_surface(cr_flip, sfc_back, 0, 0);
      cairo_paint(cr_flip);
      XFlush(disp);
      dirty = 0;
    }

    int in_overview = overview_active;
    if (s->slides[current].has_anim || elapsed < TRANSITION_DEFAULT_MS ||
        in_overview)
      usleep(16000);
    else
      usleep(50000);
  }

  cairo_destroy(cr);
  cairo_destroy(cr_flip);
  cairo_surface_destroy(sfc_back);
  cairo_surface_destroy(sfc_screen);
  XCloseDisplay(disp);

  return 0;
}
