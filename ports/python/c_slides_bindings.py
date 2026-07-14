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

class ImgFit:
    NONE = 0
    COVER = 1
    CONTAIN = 2
    FILL = 3

class ImgAlign:
    CENTER = 0
    LEFT = 1
    RIGHT = 2

class ImgUnit:
    UNSET = 0
    PX = 1
    PCT = 2

class ImageConfig(ctypes.Structure):
    _fields_ = [
        ("opacity", ctypes.c_double),
        ("rotate", ctypes.c_double),
        ("active", ctypes.c_int),
        ("fit", ctypes.c_int),
        ("width", ctypes.c_int),
        ("height", ctypes.c_int),
        ("width_unit", ctypes.c_int),
        ("height_unit", ctypes.c_int),
        ("radius", ctypes.c_int),
        ("align", ctypes.c_int),
    ]

class SlideLine(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_int),
        ("text", ctypes.c_char * 1024),
        ("marker", ctypes.c_char * 16),
        ("cols", ctypes.POINTER(ctypes.c_char * 256)),
        ("ncols", ctypes.c_int),
        ("img_cfg", ctypes.POINTER(ImageConfig)),
    ]

class Theme(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        ("bg", ctypes.c_uint32),
        ("title", ctypes.c_uint32),
        ("sub", ctypes.c_uint32),
        ("body", ctypes.c_uint32),
        ("bullet", ctypes.c_uint32),
        ("accent", ctypes.c_uint32),
        ("num", ctypes.c_uint32),
        ("table_hdr", ctypes.c_uint32),
        ("table_row", ctypes.c_uint32),
        ("table_alt", ctypes.c_uint32),
        ("table_bdr", ctypes.c_uint32),
        ("code_bg", ctypes.c_uint32),
        ("code_txt", ctypes.c_uint32),
        ("code_kw", ctypes.c_uint32),
        ("code_com", ctypes.c_uint32),
        ("code_str", ctypes.c_uint32),
        ("code_sym", ctypes.c_uint32),
    ]

class CSlides:
    def __init__(self, lib_path):
        import platform
        # Convert path extensions and prefixes dynamically for compatibility
        if platform.system() == "Windows":
            if lib_path.endswith(".so"):
                lib_path = lib_path[:-3] + ".dll"
            if not os.path.exists(lib_path):
                dir_name = os.path.dirname(lib_path)
                base_name = os.path.basename(lib_path)
                if base_name.startswith("lib"):
                    alt_path = os.path.join(dir_name, base_name[3:])
                    if os.path.exists(alt_path):
                        lib_path = alt_path
        elif platform.system() != "Windows":
            if lib_path.endswith(".dll"):
                lib_path = lib_path[:-4] + ".so"
            if not os.path.exists(lib_path):
                dir_name = os.path.dirname(lib_path)
                base_name = os.path.basename(lib_path)
                if not base_name.startswith("lib"):
                    alt_path = os.path.join(dir_name, "lib" + base_name)
                    if os.path.exists(alt_path):
                        lib_path = alt_path

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

        # const Theme* theme_find(const char *name)
        self.lib.theme_find.argtypes = [ctypes.c_char_p]
        self.lib.theme_find.restype = ctypes.POINTER(Theme)

        # void highlighter_highlight(const char *line, const char *lang,
        #                            const Theme *theme, char *out, size_t out_size)
        self.lib.highlighter_highlight.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.POINTER(Theme),
            ctypes.c_char_p,
            ctypes.c_size_t,
        ]
        self.lib.highlighter_highlight.restype = None

        # int slider_export_jpg(Slider *s, int index, const char *path, int w, int h, int quality)
        self.lib.slider_export_jpg.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_int, ctypes.c_char_p,
            ctypes.c_int, ctypes.c_int, ctypes.c_int
        ]
        self.lib.slider_export_jpg.restype = ctypes.c_int

        # int slider_export_svg(Slider *s, int index, const char *path, int w, int h)
        self.lib.slider_export_svg.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_int
        ]
        self.lib.slider_export_svg.restype = ctypes.c_int

        # int slider_export_pdf(Slider *s, const char *path, int w, int h)
        self.lib.slider_export_pdf.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_char_p, ctypes.c_int, ctypes.c_int
        ]
        self.lib.slider_export_pdf.restype = ctypes.c_int

        # int slider_export_gif(Slider *s, const char *path, int w, int h)
        self.lib.slider_export_gif.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_char_p, ctypes.c_int, ctypes.c_int
        ]
        self.lib.slider_export_gif.restype = ctypes.c_int

        # void slider_set_color(Slider *s, const char *key, const char *hex)
        self.lib.slider_set_color.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_char_p, ctypes.c_char_p
        ]
        self.lib.slider_set_color.restype = None

        # void slider_print_notes(Slider *s, int index)
        self.lib.slider_print_notes.argtypes = [ctypes.POINTER(Slider), ctypes.c_int]
        self.lib.slider_print_notes.restype = None

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

    def theme_find(self, name):
        if name is None:
            return self.lib.theme_find(None)
        return self.lib.theme_find(name.encode('utf-8'))

    def highlight(self, line, theme):
        out = ctypes.create_string_buffer(4096)
        self.lib.highlighter_highlight(
            line.encode('utf-8'), None, theme, out, len(out)
        )
        return out.value.decode('utf-8')

    def export_jpg(self, slider, index, path, w=1080, h=1080, quality=90):
        return self.lib.slider_export_jpg(slider, index, path.encode('utf-8'), w, h, quality)

    def export_svg(self, slider, index, path, w=1080, h=1080):
        return self.lib.slider_export_svg(slider, index, path.encode('utf-8'), w, h)

    def export_pdf(self, slider, path, w=1080, h=1080):
        return self.lib.slider_export_pdf(slider, path.encode('utf-8'), w, h)

    def export_gif(self, slider, path, w=1080, h=1080):
        return self.lib.slider_export_gif(slider, path.encode('utf-8'), w, h)

    def set_color(self, slider, key, hex_color):
        self.lib.slider_set_color(slider, key.encode('utf-8'), hex_color.encode('utf-8'))

    def print_notes(self, slider, index):
        self.lib.slider_print_notes(slider, index)

    def parse_line(self, line):
        sl = SlideLine()
        self.lib.parse_line(line.encode('utf-8'), ctypes.byref(sl))
        return sl

    def md_to_markup(self, text):
        out = ctypes.create_string_buffer(4096)
        self.lib.md_to_markup(text.encode('utf-8'), out, len(out))
        return out.value.decode('utf-8')
