#include "backend.h"
#include "../core/internal.h"
#include <windows.h>
#include <cairo/cairo-win32.h>
#include <stdio.h>
#include <sys/stat.h>

static Slider *g_slider = NULL;
static int g_current_slide = 0;
static int g_n_slides = 0;
static int g_fullscreen = 0;
static DWORD g_start_time = 0;
static DWORD g_slide_start_time = 0;
static int g_last_printed_slide = -1;

// Estructura para recordar la posición de la ventana antes de ir a fullscreen
static RECT g_prev_rect;
static DWORD g_prev_style;

static double get_slide_time_ms(void) {
    if (g_slide_start_time == 0) g_slide_start_time = GetTickCount();
    return (double)(GetTickCount() - g_slide_start_time);
}

static void toggle_fullscreen(HWND hwnd) {
    if (!g_fullscreen) {
        g_prev_style = GetWindowLong(hwnd, GWL_STYLE);
        GetWindowRect(hwnd, &g_prev_rect);
        
        // Monitor actual
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(monitor, &mi);

        SetWindowLong(hwnd, GWL_STYLE, g_prev_style & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(hwnd, HWND_TOP, 
                     mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        g_fullscreen = 1;
    } else {
        SetWindowLong(hwnd, GWL_STYLE, g_prev_style);
        SetWindowPos(hwnd, NULL, 
                     g_prev_rect.left, g_prev_rect.top,
                     g_prev_rect.right - g_prev_rect.left,
                     g_prev_rect.bottom - g_prev_rect.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        g_fullscreen = 0;
    }
}

static void set_transition(int new_slide) {
    if (g_slider && new_slide != g_current_slide) {
        g_slider->transition_from = g_current_slide;
        g_slider->transition_type = g_slider->slides[new_slide].transition;
    }
}

static void navigate_to(int new_slide, int *dirty, HWND hwnd) {
    if (new_slide < 0) new_slide = 0;
    if (new_slide >= g_n_slides) new_slide = g_n_slides - 1;
    if (new_slide != g_current_slide) {
        set_transition(new_slide);
        g_current_slide = new_slide;
        g_slide_start_time = GetTickCount();
        if (dirty) *dirty = 1;
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

static void render_frame(HDC hdc, int w, int h) {
    if (!g_slider) return;
    cairo_surface_t *surface = cairo_win32_surface_create(hdc);
    cairo_t *cr = cairo_create(surface);

    const Theme *t = g_slider->theme;
    cairo_set_source_rgb(cr, t->bg_r, t->bg_g, t->bg_b);
    cairo_paint(cr);

    slider_render(g_slider, g_current_slide, cr, w, h, get_slide_time_ms());

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int dirty = 0;
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        
        // Double buffering básico para evitar parpadeo
        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP membmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        SelectObject(memdc, membmp);
        
        render_frame(memdc, rect.right, rect.bottom);
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, memdc, 0, 0, SRCCOPY);
        
        DeleteObject(membmp);
        DeleteDC(memdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_TIMER: {
        static DWORD last_check_time = 0;
        DWORD now_time = GetTickCount();
        if (now_time - last_check_time > 500) {
            last_check_time = now_time;
            struct stat st;
            if (g_slider && stat(g_slider->filepath, &st) == 0) {
                if ((long long)st.st_mtime > g_slider->last_mtime) {
                    Slider *new_s = slider_load(g_slider->filepath);
                    if (new_s) {
                        // Conservar propiedades de CLI
                        new_s->theme = g_slider->theme;
                        strncpy(new_s->font_family, g_slider->font_family, sizeof(new_s->font_family) - 1);
                        new_s->font_scale = g_slider->font_scale;

                        slider_free(g_slider);
                        g_slider = new_s;
                        g_n_slides = slider_get_count(new_s);
                        if (g_current_slide >= g_n_slides) g_current_slide = g_n_slides - 1;
                        
                        dirty = 1;
                        // Forzar refresco de notas
                        g_last_printed_slide = -1;
                        fprintf(stderr, "[slides] Diapositivas recargadas automáticamente (hot reload)\n");
                    }
                }
            }
        }
        if (g_slider) {
            // Redibujar durante animaciones GIF o transiciones activas
            int in_transition = (g_slider->transition_type != TRANS_NONE &&
                GetTickCount() - g_slide_start_time < TRANSITION_DEFAULT_MS);
            if (g_slider->slides[g_current_slide].has_anim || in_transition) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }
    case WM_LBUTTONDOWN:
        navigate_to(g_current_slide - 1, &dirty, hwnd);
        break;
    case WM_RBUTTONDOWN:
        navigate_to(g_current_slide + 1, &dirty, hwnd);
        break;
    case WM_MOUSEWHEEL: {
        short delta = (short)HIWORD(wParam);
        if (delta > 0) {
            navigate_to(g_current_slide - 1, &dirty, hwnd);
        } else if (delta < 0) {
            navigate_to(g_current_slide + 1, &dirty, hwnd);
        }
        break;
    }
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_ESCAPE: PostQuitMessage(0); return 0;
        case 'Q': PostQuitMessage(0); return 0;
        case VK_RIGHT: case VK_SPACE: case VK_RETURN: case VK_NEXT:
            navigate_to(g_current_slide + 1, &dirty, hwnd);
            break;
        case VK_LEFT: case VK_BACK: case VK_PRIOR:
            navigate_to(g_current_slide - 1, &dirty, hwnd);
            break;
        case VK_HOME: navigate_to(0, &dirty, hwnd); break;
        case VK_END: navigate_to(g_n_slides - 1, &dirty, hwnd); break;
        case 'F': case VK_F11:
            toggle_fullscreen(hwnd);
            dirty = 1;
            break;
        }
        break;
    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    if (dirty) {
        if (g_slider && g_current_slide != g_last_printed_slide) {
            slider_print_notes(g_slider, g_current_slide);
            g_last_printed_slide = g_current_slide;
        }
        InvalidateRect(hwnd, NULL, FALSE);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int backend_run(Slider *s) {
    if (!s) return 1;
    g_slider = s;
    g_n_slides = slider_get_count(s);
    g_current_slide = 0;
    g_start_time = GetTickCount();
    g_slide_start_time = g_start_time;

    const char CLASS_NAME[] = "CSlidesWinClass";
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) return 1;

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "C-Slides (Win32)",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, WIN_W, WIN_H,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 1;

    // Timer para animaciones (16ms ~ 60fps)
    SetTimer(hwnd, 1, 16, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    KillTimer(hwnd, 1);

    return 0;
}
