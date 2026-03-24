import ctypes
import os

class Slider(ctypes.Structure):
    pass

class LineType:
    EMPTY = 0
    TITLE = 1
    SUBTITLE = 2
    BODY = 3
    BULLET1 = 4
    BULLET2 = 5
    IMAGE = 6
    TABLE_ROW = 7
    TABLE_SEP = 8
    BLOCKQUOTE = 9
    CODE_START = 10
    CODE_END = 11
    CODE = 12
    TASK_UNCHECKED = 13
    TASK_CHECKED = 14
    NUM_LIST = 15

class SlideLine(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_int),
        ("text", ctypes.c_char * 1024),
        ("marker", ctypes.c_char * 16),
        ("cols", (ctypes.c_char * 256) * 16),
        ("ncols", ctypes.c_int),
    ]

class Theme(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        # Colores como dobles (bg, title, sub, body, bullet, accent, num, table_hdr, table_row, table_alt, table_bdr, code_bg, code_txt, code_kw, code_com, code_str, code_sym)
        # Cada uno tiene 3 componentes (r, g, b)
        ("bg_r", ctypes.c_double), ("bg_g", ctypes.c_double), ("bg_b", ctypes.c_double),
        ("title_r", ctypes.c_double), ("title_g", ctypes.c_double), ("title_b", ctypes.c_double),
        ("sub_r", ctypes.c_double), ("sub_g", ctypes.c_double), ("sub_b", ctypes.c_double),
        ("body_r", ctypes.c_double), ("body_g", ctypes.c_double), ("body_b", ctypes.c_double),
        ("bullet_r", ctypes.c_double), ("bullet_g", ctypes.c_double), ("bullet_b", ctypes.c_double),
        ("accent_r", ctypes.c_double), ("accent_g", ctypes.c_double), ("accent_b", ctypes.c_double),
        ("num_r", ctypes.c_double), ("num_g", ctypes.c_double), ("num_b", ctypes.c_double),
        ("table_hdr_r", ctypes.c_double), ("table_hdr_g", ctypes.c_double), ("table_hdr_b", ctypes.c_double),
        ("table_row_r", ctypes.c_double), ("table_row_g", ctypes.c_double), ("table_row_b", ctypes.c_double),
        ("table_alt_r", ctypes.c_double), ("table_alt_g", ctypes.c_double), ("table_alt_b", ctypes.c_double),
        ("table_bdr_r", ctypes.c_double), ("table_bdr_g", ctypes.c_double), ("table_bdr_b", ctypes.c_double),
        ("code_bg_r", ctypes.c_double), ("code_bg_g", ctypes.c_double), ("code_bg_b", ctypes.c_double),
        ("code_txt_r", ctypes.c_double), ("code_txt_g", ctypes.c_double), ("code_txt_b", ctypes.c_double),
        ("code_kw_r", ctypes.c_double), ("code_kw_g", ctypes.c_double), ("code_kw_b", ctypes.c_double),
        ("code_com_r", ctypes.c_double), ("code_com_g", ctypes.c_double), ("code_com_b", ctypes.c_double),
        ("code_str_r", ctypes.c_double), ("code_str_g", ctypes.c_double), ("code_str_b", ctypes.c_double),
        ("code_sym_r", ctypes.c_double), ("code_sym_g", ctypes.c_double), ("code_sym_b", ctypes.c_double),
    ]

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

        # const Theme* theme_default(void)
        self.lib.theme_default.argtypes = []
        self.lib.theme_default.restype = ctypes.POINTER(Theme)

        # void highlighter_highlight(const char *line, const Theme *theme, char *out, size_t out_size)
        self.lib.highlighter_highlight.argtypes = [
            ctypes.c_char_p, ctypes.POINTER(Theme), ctypes.c_char_p, ctypes.c_size_t
        ]
        self.lib.highlighter_highlight.restype = None

        # void parse_line(const char *raw, SlideLine *out)
        self.lib.parse_line.argtypes = [ctypes.c_char_p, ctypes.POINTER(SlideLine)]
        self.lib.parse_line.restype = None

        # void md_to_markup(const char *in, char *out, size_t out_size)
        self.lib.md_to_markup.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_size_t]
        self.lib.md_to_markup.restype = None

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

    def theme_default(self):
        return self.lib.theme_default()

    def highlight(self, line, theme):
        out = ctypes.create_string_buffer(4096)
        self.lib.highlighter_highlight(line.encode('utf-8'), theme, out, len(out))
        return out.value.decode('utf-8')

    def parse_line(self, line):
        sl = SlideLine()
        self.lib.parse_line(line.encode('utf-8'), ctypes.byref(sl))
        return sl

    def md_to_markup(self, text):
        out = ctypes.create_string_buffer(4096)
        self.lib.md_to_markup(text.encode('utf-8'), out, len(out))
        return out.value.decode('utf-8')
