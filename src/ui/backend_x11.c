#include "../core/internal.h" // Para WIN_W, WIN_H, etc.
#include "backend.h"
#include "filepicker.h"
#include "tabs.h"
#include "thememenu.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cairo/cairo-xlib.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define OVERVIEW_COLS 4
#define DOUBLE_CLICK_MS 400.0

typedef enum { MODE_PRESENTER, MODE_PICKER } Mode;

static double get_time_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static const char *base_name(const char *p) {
  const char *s = strrchr(p, '/');
  return s ? s + 1 : p;
}

// Título en UTF-8 (_NET_WM_NAME) con respaldo ASCII en WM_NAME
static void set_title(Display *disp, Window win, const char *name) {
  char title[700];
  snprintf(title, sizeof(title), "C-Slides \xE2\x80\x94 %s", name);
  XStoreName(disp, win, "C-Slides");
  Atom net_name = XInternAtom(disp, "_NET_WM_NAME", False);
  Atom utf8 = XInternAtom(disp, "UTF8_STRING", False);
  XChangeProperty(disp, win, net_name, utf8, 8, PropModeReplace,
                  (unsigned char *)title, (int)strlen(title));
}

// Cambia de pestaña: reinicia temporización para no disparar kiosk/transiciones
static void activate_tab(TabSet *ts, int idx) {
  if (idx < 0 || idx >= ts->n)
    return;
  ts->active = idx;
  Tab *t = &ts->tabs[idx];
  t->slide_start_time = get_time_ms();
  t->s->transition_type = TRANS_NONE;
  t->last_printed_slide = -1;
}

// Recarga en caliente de una pestaña si el archivo cambió
static int reload_tab(Tab *t) {
  Slider *s = t->s;
  struct stat st;
  if (stat(s->filepath, &st) != 0 || (long long)st.st_mtime <= s->last_mtime)
    return 0;
  Slider *new_s = slider_load(s->filepath);
  if (!new_s)
    return 0;
  // Conservar propiedades pasadas por CLI
  new_s->theme_storage = s->theme_storage;
  new_s->theme = &new_s->theme_storage;
  strncpy(new_s->font_family, s->font_family, sizeof(new_s->font_family) - 1);
  new_s->font_scale = s->font_scale;
  new_s->kiosk_interval_ms = s->kiosk_interval_ms;
  new_s->hide_num = s->hide_num;
  slider_free(s);
  t->s = new_s;
  int n = slider_get_count(new_s);
  if (t->current >= n)
    t->current = n - 1;
  if (t->current < 0)
    t->current = 0;
  t->last_printed_slide = -1; // Forzar refresco de notas en consola
  fprintf(stderr, "[slides] Diapositivas recargadas automáticamente "
                  "(hot reload)\n");
  return 1;
}

// Directorio inicial del selector: carpeta del archivo activo, o NULL (cwd)
static void picker_start_dir(TabSet *ts, char *out, size_t n) {
  out[0] = '\0';
  Tab *t = tabs_active(ts);
  if (!t)
    return;
  char real[PATH_MAX];
  if (!realpath(t->s->filepath, real))
    return;
  char *slash = strrchr(real, '/');
  if (!slash)
    return;
  if (slash == real)
    slash[1] = '\0';
  else
    *slash = '\0';
  snprintf(out, n, "%s", real);
}

static void go_prev(Tab *t, Slider *s) {
  if (t->current > 0) {
    s->transition_from = t->current;
    s->transition_type = s->slides[t->current - 1].transition;
    t->current--;
    t->slide_start_time = get_time_ms();
  }
}

static void go_next(Tab *t, Slider *s, int n_slides) {
  if (t->current < n_slides - 1) {
    s->transition_from = t->current;
    s->transition_type = s->slides[t->current + 1].transition;
    t->current++;
    t->slide_start_time = get_time_ms();
  }
}

int backend_run(Slider **initial, int n_initial, void (*configure)(Slider *)) {
  TabSet ts;
  tabs_init(&ts);
  for (int i = 0; i < n_initial; i++)
    if (initial && initial[i])
      tabs_add(&ts, initial[i], get_time_ms());
  if (ts.n > 0)
    ts.active = 0;

  setlocale(LC_CTYPE, "");
  Display *disp = XOpenDisplay(NULL);
  if (!disp) {
    fputs("No se pudo abrir display X11\n", stderr);
    tabs_free(&ts);
    return 1;
  }

  int scr = DefaultScreen(disp);
  int sw = DisplayWidth(disp, scr);
  int sh = DisplayHeight(disp, scr);

  int wx = (sw - WIN_W) / 2;
  int wy = (sh - WIN_H) / 2;
  Window win = XCreateSimpleWindow(disp, RootWindow(disp, scr), wx, wy, WIN_W,
                                   WIN_H, 0, 0, 0);

  XSelectInput(disp, win,
               KeyPressMask | ButtonPressMask | StructureNotifyMask |
                   ExposureMask);
  Atom wm_delete = XInternAtom(disp, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(disp, win, &wm_delete, 1);
  Atom wm_state = XInternAtom(disp, "_NET_WM_STATE", False);
  Atom fs_atom = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);
  XMapWindow(disp, win);

  // Entrada de texto UTF-8 (filtro del selector); si falla, XLookupString
  XIM xim = NULL;
  XIC xic = NULL;
  if (XSetLocaleModifiers("") != NULL)
    xim = XOpenIM(disp, NULL, NULL, NULL);
  if (xim) {
    xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                    XNClientWindow, win, XNFocusWindow, win, NULL);
  }

  int win_w = WIN_W, win_h = WIN_H;
  cairo_surface_t *sfc_screen = cairo_xlib_surface_create(
      disp, win, DefaultVisual(disp, scr), win_w, win_h);
  cairo_surface_t *sfc_back = cairo_surface_create_similar(
      sfc_screen, CAIRO_CONTENT_COLOR, win_w, win_h);
  cairo_t *cr = cairo_create(sfc_back);
  cairo_t *cr_flip = cairo_create(sfc_screen);

  int running = 1, dirty = 1, fullscreen = 0;
  Mode mode = MODE_PRESENTER;
  FilePicker fp;
  int fp_ready = 0;
  int title_dirty = 1;
  double last_click_time = 0;
  int last_click_x = -100, last_click_y = -100;
  int exit_code = 0;

  if (ts.n == 0) {
    fp_init(&fp, NULL);
    fp_ready = 1;
    mode = MODE_PICKER;
  }

  double last_check_time = 0; // Para controlar el intervalo de recarga

  while (running) {
    while (XPending(disp)) {
      XEvent ev;
      XNextEvent(disp, &ev);
      if (XFilterEvent(&ev, None))
        continue;

      Tab *t = tabs_active(&ts);
      Slider *s = t ? t->s : NULL;
      int n_slides = s ? slider_get_count(s) : 0;
      int bar_h = (mode == MODE_PRESENTER && !fullscreen && ts.n >= 2)
                      ? TAB_BAR_H : 0;
      int view_h = win_h - bar_h;

      if (ev.type == KeyPress && mode == MODE_PICKER) {
        KeySym ks = XLookupKeysym(&ev.xkey, 0);
        int ctrl = (ev.xkey.state & ControlMask) != 0;
        FPResult r = FP_NONE;
        if (ctrl && (ks == XK_h || ks == XK_H)) {
          r = fp_key(&fp, FPK_TOGGLE_HIDDEN, NULL);
        } else if (ks == XK_Up || ks == XK_KP_Up) r = fp_key(&fp, FPK_UP, NULL);
        else if (ks == XK_Down || ks == XK_KP_Down) r = fp_key(&fp, FPK_DOWN, NULL);
        else if (ks == XK_Prior || ks == XK_KP_Prior) r = fp_key(&fp, FPK_PAGEUP, NULL);
        else if (ks == XK_Next || ks == XK_KP_Next) r = fp_key(&fp, FPK_PAGEDOWN, NULL);
        else if (ks == XK_Home) r = fp_key(&fp, FPK_HOME, NULL);
        else if (ks == XK_End) r = fp_key(&fp, FPK_END, NULL);
        else if (ks == XK_Return || ks == XK_KP_Enter) r = fp_key(&fp, FPK_ENTER, NULL);
        else if (ks == XK_BackSpace) r = fp_key(&fp, FPK_BACKSPACE, NULL);
        else if (ks == XK_Escape) r = fp_key(&fp, FPK_ESCAPE, NULL);
        else if (!ctrl) {
          char buf[64] = {0};
          int len = 0;
          Status st;
          if (xic)
            len = Xutf8LookupString(xic, &ev.xkey, buf, sizeof(buf) - 1, NULL, &st);
          else
            len = XLookupString(&ev.xkey, buf, sizeof(buf) - 1, NULL, NULL);
          if (len > 0) {
            buf[len] = '\0';
            r = fp_key(&fp, FPK_TEXT, buf);
          }
        }
        if (r == FP_REDRAW)
          dirty = 1;
        else if (r == FP_CANCEL) {
          if (ts.n > 0) {
            mode = MODE_PRESENTER;
            title_dirty = dirty = 1;
          } else {
            running = 0;
          }
        } else if (r == FP_CHOSEN) {
          Slider *ns = slider_load(fp_chosen_path(&fp));
          if (!ns) {
            char msg[PATH_MAX + 64];
            snprintf(msg, sizeof(msg), "No se pudo cargar: %s",
                     base_name(fp_chosen_path(&fp)));
            fp_set_status(&fp, msg);
            dirty = 1;
          } else {
            if (configure)
              configure(ns);
            if (tabs_add(&ts, ns, get_time_ms()) < 0) {
              fp_set_status(&fp, "Demasiadas pestañas abiertas");
            } else {
              activate_tab(&ts, ts.active);
              mode = MODE_PRESENTER;
            }
            title_dirty = dirty = 1;
          }
        }
        continue;
      }

      if (ev.type == KeyPress && t) {
        KeySym ks = XLookupKeysym(&ev.xkey, 0);
        int ctrl = (ev.xkey.state & ControlMask) != 0;
        int shift = (ev.xkey.state & ShiftMask) != 0;
        // Atajos de pestañas (antes de Tab / PageUp / PageDown)
        if (ctrl && (ks == XK_o || ks == XK_O)) {
          char dir[PATH_MAX];
          picker_start_dir(&ts, dir, sizeof(dir));
          if (fp_ready)
            fp_free(&fp);
          fp_init(&fp, dir[0] ? dir : NULL);
          fp_ready = 1;
          mode = MODE_PICKER;
          title_dirty = dirty = 1;
        } else if (ctrl && (ks == XK_w || ks == XK_W)) {
          tabs_close(&ts, ts.active);
          if (ts.n == 0) {
            if (fp_ready)
              fp_free(&fp);
            fp_init(&fp, NULL);
            fp_ready = 1;
            mode = MODE_PICKER;
          } else {
            activate_tab(&ts, ts.active);
          }
          title_dirty = dirty = 1;
        } else if (ctrl && ((ks == XK_Tab && !shift) || ks == XK_Next)) {
          tabs_next(&ts);
          activate_tab(&ts, ts.active);
          title_dirty = dirty = 1;
        } else if (ctrl && ((ks == XK_Tab && shift) || ks == XK_ISO_Left_Tab ||
                            ks == XK_Prior)) {
          tabs_prev(&ts);
          activate_tab(&ts, ts.active);
          title_dirty = dirty = 1;
        } else if (ctrl && ks >= XK_1 && ks <= XK_9) {
          int idx = (int)(ks - XK_1);
          if (idx < ts.n) {
            activate_tab(&ts, idx);
            title_dirty = dirty = 1;
          }
        } else if (t->theme_menu) {
          // Menú de temas abierto: captura la navegación
          if (ks == XK_Escape || ks == XK_t || ks == XK_T) {
            t->theme_menu = 0;
            dirty = 1;
          } else if (ks == XK_Return || ks == XK_KP_Enter || ks == XK_space) {
            thememenu_apply(t, t->theme_sel);
            dirty = 1;
          } else if (ks == XK_Right || ks == XK_Tab) {
            dirty |= thememenu_move(t, 1, 0);
          } else if (ks == XK_Left || ks == XK_ISO_Left_Tab) {
            dirty |= thememenu_move(t, -1, 0);
          } else if (ks == XK_Down) {
            dirty |= thememenu_move(t, 0, 1);
          } else if (ks == XK_Up) {
            dirty |= thememenu_move(t, 0, -1);
          } else if (ks == XK_Home) {
            t->theme_sel = 0;
            dirty = 1;
          } else if (ks == XK_End) {
            t->theme_sel = thememenu_count(t) - 1;
            dirty = 1;
          } else if (ks == XK_q) {
            running = 0;
          }
        } else if (ks == XK_t || ks == XK_T) {
          t->overview = 0;
          thememenu_open(t);
          dirty = 1;
        } else if (ks == XK_Tab) {
          t->overview = !t->overview;
          if (t->overview)
            s->transition_type = TRANS_NONE;
          dirty = 1;
        } else if (t->overview) {
          if (ks == XK_Escape) {
            t->overview = 0;
            s->transition_type = TRANS_NONE;
            dirty = 1;
          } else if (ks == XK_Right || ks == XK_Down) {
            if (t->current < n_slides - 1) {
              t->current++;
              dirty = 1;
            }
          } else if (ks == XK_Left || ks == XK_Up) {
            if (t->current > 0) {
              t->current--;
              dirty = 1;
            }
          } else if (ks == XK_Home) {
            t->current = 0;
            dirty = 1;
          } else if (ks == XK_End) {
            t->current = n_slides - 1;
            dirty = 1;
          } else if (ks == XK_Return || ks == XK_space) {
            t->overview = 0;
            s->transition_type = TRANS_NONE;
            t->slide_start_time = get_time_ms();
            dirty = 1;
          }
        } else if ((ks == XK_plus || ks == XK_equal || ks == XK_KP_Add) && ctrl) {
          t->zoom *= 1.2;
          if (t->zoom > 5.0)
            t->zoom = 5.0;
          dirty = 1;
        } else if ((ks == XK_minus || ks == XK_underscore ||
                    ks == XK_KP_Subtract) && ctrl) {
          t->zoom /= 1.2;
          if (t->zoom < 0.3)
            t->zoom = 0.3;
          dirty = 1;
        } else if (ks == XK_0 && ctrl) {
          t->zoom = 1.0;
          dirty = 1;
        } else if (ks == XK_q || ks == XK_Escape)
          running = 0;
        else if (ks == XK_Right || ks == XK_Return || ks == XK_space ||
                 ks == XK_Next) {
          go_next(t, s, n_slides);
          dirty = 1;
        } else if (ks == XK_Left || ks == XK_BackSpace || ks == XK_Prior) {
          go_prev(t, s);
          dirty = 1;
        } else if (ks == XK_Home) {
          s->transition_from = t->current;
          s->transition_type = s->slides[0].transition;
          t->current = 0;
          dirty = 1;
          t->slide_start_time = get_time_ms();
        } else if (ks == XK_End) {
          s->transition_from = t->current;
          s->transition_type = s->slides[n_slides - 1].transition;
          t->current = n_slides - 1;
          dirty = 1;
          t->slide_start_time = get_time_ms();
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

      if (ev.type == ButtonPress && mode == MODE_PICKER) {
        FPResult r = FP_NONE;
        int b = ev.xbutton.button;
        if (b == Button1) {
          double now = get_time_ms();
          int dbl = (now - last_click_time < DOUBLE_CLICK_MS &&
                     abs(ev.xbutton.x - last_click_x) < 5 &&
                     abs(ev.xbutton.y - last_click_y) < 5);
          last_click_time = dbl ? 0 : now;
          last_click_x = ev.xbutton.x;
          last_click_y = ev.xbutton.y;
          r = fp_click(&fp, ev.xbutton.x, ev.xbutton.y, dbl);
        } else if (b == Button4) r = fp_scroll(&fp, -3);
        else if (b == Button5) r = fp_scroll(&fp, 3);
        if (r == FP_REDRAW)
          dirty = 1;
        else if (r == FP_CHOSEN) {
          // Reutiliza la ruta de carga del teclado simulando Enter sobre la selección
          Slider *ns = slider_load(fp_chosen_path(&fp));
          if (!ns) {
            fp_set_status(&fp, "No se pudo cargar el archivo");
          } else {
            if (configure)
              configure(ns);
            if (tabs_add(&ts, ns, get_time_ms()) < 0) {
              fp_set_status(&fp, "Demasiadas pestañas abiertas");
            } else {
              activate_tab(&ts, ts.active);
              mode = MODE_PRESENTER;
            }
          }
          title_dirty = dirty = 1;
        }
      } else if (ev.type == ButtonPress && t) {
        int bx = ev.xbutton.x, by = ev.xbutton.y;
        if (by < bar_h) {
          if (ev.xbutton.button == Button1) {
            TabHit h = tabs_hit_test(&ts, win_w, bar_h, bx, by);
            if (h.kind == TAB_HIT_TAB) {
              activate_tab(&ts, h.index);
            } else if (h.kind == TAB_HIT_CLOSE) {
              tabs_close(&ts, h.index);
              if (ts.n == 0) {
                if (fp_ready)
                  fp_free(&fp);
                fp_init(&fp, NULL);
                fp_ready = 1;
                mode = MODE_PICKER;
              } else {
                activate_tab(&ts, ts.active);
              }
            } else if (h.kind == TAB_HIT_PLUS) {
              char dir[PATH_MAX];
              picker_start_dir(&ts, dir, sizeof(dir));
              if (fp_ready)
                fp_free(&fp);
              fp_init(&fp, dir[0] ? dir : NULL);
              fp_ready = 1;
              mode = MODE_PICKER;
            }
            title_dirty = dirty = 1;
          }
        } else if (t->theme_menu) {
          int b = ev.xbutton.button;
          if (b == Button1) {
            int idx = thememenu_hit(t, win_w, view_h, bx, by - bar_h);
            if (idx >= 0)
              thememenu_apply(t, idx);
          } else if (b == Button3) {
            t->theme_menu = 0;
          } else if (b == Button4) {
            thememenu_move(t, 0, -1);
          } else if (b == Button5) {
            thememenu_move(t, 0, 1);
          }
          dirty = 1;
        } else if (t->overview) {
          int cols = OVERVIEW_COLS;
          double tw = (double)win_w / cols;
          double th = (double)view_h / ((n_slides + cols - 1) / cols);
          int col = bx / tw;
          int oy = by - bar_h;
          int row = (oy - 30) / (th - 30);
          if (oy < 30)
            row = -1;
          int idx = row * cols + col;
          if (row >= 0 && col >= 0 && col < cols && idx >= 0 &&
              idx < n_slides) {
            if (idx != t->current) {
              t->current = idx;
              t->slide_start_time = get_time_ms();
            }
          }
          t->overview = 0;
          s->transition_type = TRANS_NONE;
          dirty = 1;
        } else if (ev.xbutton.button == Button1 ||
                   ev.xbutton.button == Button4) {
          go_prev(t, s);
          dirty = 1;
        } else if (ev.xbutton.button == Button3 ||
                   ev.xbutton.button == Button5) {
          go_next(t, s, n_slides);
          dirty = 1;
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

    if (!running)
      break;

    // Título de ventana
    if (title_dirty) {
      Tab *at = tabs_active(&ts);
      if (mode == MODE_PICKER || !at)
        set_title(disp, win, "Abrir archivo");
      else
        set_title(disp, win, base_name(at->s->filepath));
      title_dirty = 0;
    }

    double now_time = get_time_ms();
    if (now_time - last_check_time > 500.0) { // Hot reload de todas las pestañas
      last_check_time = now_time;
      for (int i = 0; i < ts.n; i++)
        if (reload_tab(&ts.tabs[i]) && i == ts.active)
          dirty = 1;
    }

    Tab *t = tabs_active(&ts);
    int animating = 0;

    if (mode == MODE_PRESENTER && t) {
      Slider *s = t->s;
      int n_slides = slider_get_count(s);
      double elapsed = get_time_ms() - t->slide_start_time;
      animating = s->slides[t->current].has_anim ||
                  elapsed < TRANSITION_DEFAULT_MS || t->overview;
      if (animating)
        dirty = 1;
      // Avance automático (modo kiosk)
      if (s->kiosk_interval_ms > 0 && !t->overview && !t->theme_menu &&
          elapsed >= (double)s->kiosk_interval_ms) {
        int next = t->current + 1;
        if (next >= n_slides)
          next = 0;
        s->transition_from = t->current;
        s->transition_type = s->slides[next].transition;
        t->current = next;
        t->slide_start_time = get_time_ms();
        dirty = 1;
      }
    }

    if (dirty) {
      if (mode == MODE_PICKER || !t) {
        fp_render(&fp, cr, win_w, win_h);
      } else {
        Slider *s = t->s;
        int n_slides = slider_get_count(s);
        int bar_h = (!fullscreen && ts.n >= 2) ? TAB_BAR_H : 0;
        int view_h = win_h - bar_h;

        if (!t->overview && t->current != t->last_printed_slide) {
          slider_print_notes(s, t->current);
          t->last_printed_slide = t->current;
        }
        double now = get_time_ms() - t->slide_start_time;
        cairo_set_source_rgb(cr, COLOR_R(s->theme->bg), COLOR_G(s->theme->bg),
                             COLOR_B(s->theme->bg));
        cairo_paint(cr);

        cairo_save(cr);
        cairo_translate(cr, 0, bar_h);
        cairo_rectangle(cr, 0, 0, win_w, view_h);
        cairo_clip(cr);

        if (t->theme_menu) {
          thememenu_render(t, cr, win_w, view_h);
        } else if (t->overview) {
          int n = n_slides;
          int cols = OVERVIEW_COLS;
          int rows = (n + cols - 1) / cols;
          double tw = (double)win_w / cols;
          double th = (double)view_h / rows;

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
            double sy = (cell_h - 20) / view_h;
            double scl = (sx < sy) ? sx : sy;
            double ox = cx + (tw - win_w * scl) / 2;
            double oy = cy + 4;

            cairo_save(cr);
            cairo_translate(cr, ox, oy);
            cairo_scale(cr, scl, scl);
            cairo_rectangle(cr, 0, 0, win_w, view_h);
            cairo_clip(cr);
            slider_render(s, i, cr, win_w, view_h, 0.0);
            cairo_restore(cr);

            cairo_rectangle(cr, cx + 2, cy + 2, tw - 4, cell_h - 4);
            if (i == t->current) {
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
          if (t->zoom != 1.0) {
            cairo_translate(cr, win_w / 2.0, view_h / 2.0);
            cairo_scale(cr, t->zoom, t->zoom);
            cairo_translate(cr, -win_w / 2.0, -view_h / 2.0);
          }
          slider_render(s, t->current, cr, win_w, view_h, now);
          cairo_restore(cr);
        }
        cairo_restore(cr);

        if (bar_h > 0)
          tabs_render_bar(&ts, cr, win_w, bar_h);
      }

      cairo_set_source_surface(cr_flip, sfc_back, 0, 0);
      cairo_paint(cr_flip);
      XFlush(disp);
      dirty = 0;
    }

    if (animating)
      usleep(16000);
    else
      usleep(50000);
  }

  if (fp_ready)
    fp_free(&fp);
  tabs_free(&ts);
  cairo_destroy(cr);
  cairo_destroy(cr_flip);
  cairo_surface_destroy(sfc_back);
  cairo_surface_destroy(sfc_screen);
  if (xic)
    XDestroyIC(xic);
  if (xim)
    XCloseIM(xim);
  XCloseDisplay(disp);

  return exit_code;
}
