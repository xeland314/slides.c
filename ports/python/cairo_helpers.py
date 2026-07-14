import ctypes
import os
import platform
import tempfile

_cairo = None

def _load_cairo():
    global _cairo
    if _cairo is not None:
        return _cairo
    if platform.system() == "Windows":
        for name in ["libcairo-2.dll", "libcairo.dll"]:
            try:
                _cairo = ctypes.CDLL(name)
                break
            except OSError:
                continue
    else:
        for name in ["libcairo.so.2", "libcairo.so"]:
            try:
                _cairo = ctypes.CDLL(name)
                break
            except OSError:
                continue
    if _cairo is None:
        raise OSError("No se pudo cargar libcairo")
    return _cairo

def create_image_surface(w, h):
    """Create a Cairo ARGB32 image surface."""
    cairo = _load_cairo()
    cairo.cairo_image_surface_create.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
    cairo.cairo_image_surface_create.restype = ctypes.c_void_p
    CAIRO_FORMAT_ARGB32 = 0
    return cairo.cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h)

def create_context(surface):
    """Create a cairo_t from a surface."""
    cairo = _load_cairo()
    cairo.cairo_create.argtypes = [ctypes.c_void_p]
    cairo.cairo_create.restype = ctypes.c_void_p
    return cairo.cairo_create(surface)

def destroy_context(cr):
    cairo = _load_cairo()
    cairo.cairo_destroy.argtypes = [ctypes.c_void_p]
    cairo.cairo_destroy(cr)

def destroy_surface(sfc):
    cairo = _load_cairo()
    cairo.cairo_surface_destroy.argtypes = [ctypes.c_void_p]
    cairo.cairo_surface_destroy(sfc)

def surface_write_png(surface, path):
    cairo = _load_cairo()
    cairo.cairo_surface_write_to_png.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    cairo.cairo_surface_write_to_png.restype = ctypes.c_int
    return cairo.cairo_surface_write_to_png(surface, path.encode('utf-8'))

def surface_status(surface):
    cairo = _load_cairo()
    cairo.cairo_surface_status.argtypes = [ctypes.c_void_p]
    cairo.cairo_surface_status.restype = ctypes.c_int
    return cairo.cairo_surface_status(surface)
