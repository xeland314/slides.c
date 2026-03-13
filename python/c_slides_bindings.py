import ctypes
import os

class Slider(ctypes.Structure):
    pass

class CSlides:
    def __init__(self, lib_path):
        if not os.path.exists(lib_path):
            raise FileNotFoundError(f"Librería no encontrada en {lib_path}")
        
        self.lib = ctypes.CDLL(lib_path)

        # Slider* slider_load(const char *path)
        self.lib.slider_load.argtypes = [ctypes.c_char_p]
        self.lib.slider_load.restype = ctypes.POINTER(Slider)

        # void slider_free(Slider *s)
        self.lib.slider_free.argtypes = [ctypes.POINTER(Slider)]
        self.lib.slider_free.restype = None

        # int slider_get_count(Slider *s)
        self.lib.slider_get_count.argtypes = [ctypes.POINTER(Slider)]
        self.lib.slider_get_count.restype = ctypes.c_int

        # int backend_run(Slider *s)
        self.lib.backend_run.argtypes = [ctypes.POINTER(Slider)]
        self.lib.backend_run.restype = ctypes.c_int

        # int slider_export_png(Slider *s, int index, const char *path, int w, int h)
        self.lib.slider_export_png.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_int
        ]
        self.lib.slider_export_png.restype = ctypes.c_int

        # void slider_set_theme(Slider *s, const char *theme_name)
        self.lib.slider_set_theme.argtypes = [ctypes.POINTER(Slider), ctypes.c_char_p]
        self.lib.slider_set_theme.restype = None

        # void slider_set_font_family(Slider *s, const char *font_family)
        self.lib.slider_set_font_family.argtypes = [ctypes.POINTER(Slider), ctypes.c_char_p]
        self.lib.slider_set_font_family.restype = None

        # void slider_set_font_scale(Slider *s, double font_scale)
        self.lib.slider_set_font_scale.argtypes = [ctypes.POINTER(Slider), ctypes.c_double]
        self.lib.slider_set_font_scale.restype = None

        # const char* slider_get_theme_name(Slider *s)
        self.lib.slider_get_theme_name.argtypes = [ctypes.POINTER(Slider)]
        self.lib.slider_get_theme_name.restype = ctypes.c_char_p

        # const char* slider_get_font_family(Slider *s)
        self.lib.slider_get_font_family.argtypes = [ctypes.POINTER(Slider)]
        self.lib.slider_get_font_family.restype = ctypes.c_char_p

        # double slider_get_font_scale(Slider *s)
        self.lib.slider_get_font_scale.argtypes = [ctypes.POINTER(Slider)]
        self.lib.slider_get_font_scale.restype = ctypes.c_double

    def load(self, path):
        return self.lib.slider_load(path.encode('utf-8'))

    def free(self, slider):
        if slider:
            self.lib.slider_free(slider)

    def get_count(self, slider):
        return self.lib.slider_get_count(slider)

    def run_backend(self, slider):
        return self.lib.backend_run(slider)

    def export_png(self, slider, index, path, w=1080, h=1080):
        return self.lib.slider_export_png(slider, index, path.encode('utf-8'), w, h)

    def set_theme(self, slider, name):
        self.lib.slider_set_theme(slider, name.encode('utf-8'))

    def set_font_family(self, slider, family):
        self.lib.slider_set_font_family(slider, family.encode('utf-8'))

    def set_font_scale(self, slider, scale):
        self.lib.slider_set_font_scale(slider, float(scale))

    def get_theme_name(self, slider):
        res = self.lib.slider_get_theme_name(slider)
        return res.decode('utf-8') if res else "unknown"

    def get_font_family(self, slider):
        res = self.lib.slider_get_font_family(slider)
        return res.decode('utf-8') if res else "sans"

    def get_font_scale(self, slider):
        return self.lib.slider_get_font_scale(slider)
