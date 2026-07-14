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

class TransitionType:
    NONE = 0
    FADE = 1
    SLIDE_LEFT = 2
    SLIDE_RIGHT = 3
    SLIDE_UP = 4
    SLIDE_DOWN = 5

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

        # void py_lexer_run(const char *line, const Theme *theme, char *out, size_t out_size)
        self.lib.py_lexer_run.argtypes = [
            ctypes.c_char_p, ctypes.POINTER(Theme), ctypes.c_char_p, ctypes.c_size_t
        ]
        self.lib.py_lexer_run.restype = None

        # void do_transition(Slider *s, int from_idx, int to_idx, cairo_t *cr,
        #                     int win_w, int win_h, double progress)
        self.lib.do_transition.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_int, ctypes.c_int, ctypes.c_void_p,
            ctypes.c_int, ctypes.c_int, ctypes.c_double
        ]
        self.lib.do_transition.restype = None

        # void slider_render(Slider *s, int index, cairo_t *cr, int win_w, int win_h, double time_ms)
        self.lib.slider_render.argtypes = [
            ctypes.POINTER(Slider), ctypes.c_int, ctypes.c_void_p,
            ctypes.c_int, ctypes.c_int, ctypes.c_double
        ]
        self.lib.slider_render.restype = None

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

    def highlight_python(self, line, theme):
        out = ctypes.create_string_buffer(4096)
        self.lib.py_lexer_run(line.encode('utf-8'), theme, out, len(out))
        return out.value.decode('utf-8')

    def highlight_lang(self, line, lang, theme):
        out = ctypes.create_string_buffer(4096)
        lang_bytes = lang.encode('utf-8') if lang else None
        self.lib.highlighter_highlight(line.encode('utf-8'), lang_bytes, theme, out, len(out))
        return out.value.decode('utf-8')

    def do_transition(self, slider, from_idx, to_idx, cr, win_w, win_h, progress):
        self.lib.do_transition(slider, from_idx, to_idx, cr, win_w, win_h, progress)

    def render_slide(self, slider, index, cr, win_w, win_h, time_ms):
        self.lib.slider_render(slider, index, cr, win_w, win_h, time_ms)

    # ── Test accessors (read internal Slider state) ────────────────────────

    def test_get_transition(self, slider, slide_idx):
        return self.lib.slider_test_get_transition(slider, slide_idx)

    def test_get_notes(self, slider, slide_idx):
        self.lib.slider_test_get_notes.restype = ctypes.c_char_p
        return self.lib.slider_test_get_notes(slider, slide_idx).decode('utf-8', errors='replace')

    def test_get_nlines(self, slider, slide_idx):
        return self.lib.slider_test_get_nlines(slider, slide_idx)

    def test_get_line_type(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_line_type(slider, slide_idx, line_idx)

    def test_get_line_text(self, slider, slide_idx, line_idx):
        self.lib.slider_test_get_line_text.restype = ctypes.c_char_p
        return self.lib.slider_test_get_line_text(slider, slide_idx, line_idx).decode('utf-8', errors='replace')

    def test_get_line_marker(self, slider, slide_idx, line_idx):
        self.lib.slider_test_get_line_marker.restype = ctypes.c_char_p
        return self.lib.slider_test_get_line_marker(slider, slide_idx, line_idx).decode('utf-8', errors='replace')

    def test_get_line_ncols(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_line_ncols(slider, slide_idx, line_idx)

    def test_get_line_col(self, slider, slide_idx, line_idx, col_idx):
        self.lib.slider_test_get_line_col.restype = ctypes.c_char_p
        return self.lib.slider_test_get_line_col(slider, slide_idx, line_idx, col_idx).decode('utf-8', errors='replace')

    def test_get_line_has_img_cfg(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_line_has_img_cfg(slider, slide_idx, line_idx)

    def test_get_img_cfg_active(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_active(slider, slide_idx, line_idx)

    def test_get_img_cfg_opacity(self, slider, slide_idx, line_idx):
        self.lib.slider_test_get_img_cfg_opacity.restype = ctypes.c_double
        return self.lib.slider_test_get_img_cfg_opacity(slider, slide_idx, line_idx)

    def test_get_img_cfg_rotate(self, slider, slide_idx, line_idx):
        self.lib.slider_test_get_img_cfg_rotate.restype = ctypes.c_double
        return self.lib.slider_test_get_img_cfg_rotate(slider, slide_idx, line_idx)

    def test_get_img_cfg_fit(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_fit(slider, slide_idx, line_idx)

    def test_get_img_cfg_width(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_width(slider, slide_idx, line_idx)

    def test_get_img_cfg_height(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_height(slider, slide_idx, line_idx)

    def test_get_img_cfg_width_unit(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_width_unit(slider, slide_idx, line_idx)

    def test_get_img_cfg_height_unit(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_height_unit(slider, slide_idx, line_idx)

    def test_get_img_cfg_radius(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_radius(slider, slide_idx, line_idx)

    def test_get_img_cfg_align(self, slider, slide_idx, line_idx):
        return self.lib.slider_test_get_img_cfg_align(slider, slide_idx, line_idx)

    def test_get_transition_type(self, slider):
        return self.lib.slider_test_get_transition_type(slider)

    def test_get_transition_from(self, slider):
        return self.lib.slider_test_get_transition_from(slider)

    def test_set_transition_type(self, slider, type_val):
        self.lib.slider_test_set_transition_type(slider, type_val)

    def test_set_transition_from(self, slider, from_val):
        self.lib.slider_test_set_transition_from(slider, from_val)

    def test_get_filepath(self, slider):
        self.lib.slider_test_get_filepath.restype = ctypes.c_char_p
        return self.lib.slider_test_get_filepath(slider).decode('utf-8', errors='replace')

    def test_get_mtime(self, slider):
        self.lib.slider_test_get_mtime.restype = ctypes.c_int64
        return self.lib.slider_test_get_mtime(slider)

    def test_get_hide_num(self, slider):
        return self.lib.slider_test_get_hide_num(slider)

    def test_get_kiosk_interval(self, slider):
        return self.lib.slider_test_get_kiosk_interval(slider)

    def test_get_line_has_anim(self, slider, slide_idx):
        return self.lib.slider_test_get_line_has_anim(slider, slide_idx)
