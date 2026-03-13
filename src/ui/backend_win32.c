#include "backend.h"
#include "../core/internal.h"
#include <windows.h>
#include <cairo/cairo-win32.h>
#include <stdio.h>

// Variable global para mantener el estado del slider
static Slider *g_slider = NULL;
static int g_current_slide = 0;
static int g_n_slides = 0;

static void render_frame(HDC hdc, int w, int h) {
    if (!g_slider) return;

    // Crear superficie Cairo sobre el DC de Windows
    cairo_surface_t *surface = cairo_win32_surface_create(hdc);
    cairo_t *cr = cairo_create(surface);

    // Fondo (usando el tema actual)
    const Theme *t = g_slider->theme; // Acceso directo al struct definido en internal.h
    cairo_set_source_rgb(cr, t->bg_r, t->bg_g, t->bg_b);
    cairo_paint(cr);

    // Renderizar slide
    slider_render(g_slider, g_current_slide, cr, w, h);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        RECT rect;
        GetClientRect(hwnd, &rect);
        render_frame(hdc, rect.right, rect.bottom);
        
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_KEYDOWN: {
        int dirty = 0;
        switch (wParam) {
        case VK_ESCAPE:
        case 'Q':
            PostQuitMessage(0);
            return 0;
        case VK_RIGHT:
        case VK_SPACE:
        case VK_RETURN:
            if (g_current_slide < g_n_slides - 1) { g_current_slide++; dirty = 1; }
            break;
        case VK_LEFT:
        case VK_BACK:
            if (g_current_slide > 0) { g_current_slide--; dirty = 1; }
            break;
        case VK_HOME:
            g_current_slide = 0; dirty = 1;
            break;
        case VK_END:
            g_current_slide = g_n_slides - 1; dirty = 1;
            break;
        }
        if (dirty) {
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int backend_run(Slider *s) {
    if (!s) return 1;
    g_slider = s;
    g_n_slides = slider_get_count(s);
    g_current_slide = 0;

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

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
