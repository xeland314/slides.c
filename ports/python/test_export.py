import unittest
import os
from c_slides_bindings import CSlides


class TestExportFormats(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)
        cls.md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        cls.out_dir = os.path.abspath(os.path.dirname(__file__))

    def _out(self, name):
        return os.path.join(self.out_dir, name)

    def _cleanup(self, path):
        if os.path.exists(path):
            os.remove(path)

    def test_export_png_basic(self):
        out = self._out("test_export_png_basic.png")
        self._cleanup(out)
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        res = self.cs.export_png(s, 0, out, 800, 600)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 1000)

        self.cs.free(s)
        self._cleanup(out)

    def test_export_png_different_resolutions(self):
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        for w, h in [(1920, 1080), (640, 480), (3840, 2160)]:
            with self.subTest(w=w, h=h):
                out = self._out(f"test_res_{w}x{h}.png")
                self._cleanup(out)
                res = self.cs.export_png(s, 0, out, w, h)
                self.assertEqual(res, 0)
                self.assertTrue(os.path.exists(out))
                self.assertGreater(os.path.getsize(out), 500)
                self._cleanup(out)

        self.cs.free(s)

    def test_export_jpg_basic(self):
        out = self._out("test_export_jpg_basic.jpg")
        self._cleanup(out)
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        res = self.cs.export_jpg(s, 0, out, 800, 600, 90)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 1000)

        self.cs.free(s)
        self._cleanup(out)

    def test_export_jpg_low_quality(self):
        out = self._out("test_export_jpg_lowq.jpg")
        self._cleanup(out)
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        res = self.cs.export_jpg(s, 0, out, 800, 600, 10)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        size_low = os.path.getsize(out)

        self.cs.free(s)
        self._cleanup(out)

        # Low quality should produce a smaller file than high quality
        out_hq = self._out("test_export_jpg_hq.jpg")
        self._cleanup(out_hq)
        s = self.cs.load(self.md_path)
        res = self.cs.export_jpg(s, 0, out_hq, 800, 600, 100)
        self.assertEqual(res, 0)
        size_high = os.path.getsize(out_hq)
        self.assertGreater(size_high, size_low)

        self.cs.free(s)
        self._cleanup(out_hq)

    def test_export_svg_basic(self):
        out = self._out("test_export_svg_basic.svg")
        self._cleanup(out)
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        res = self.cs.export_svg(s, 0, out, 800, 600)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 100)

        # SVG should contain XML header
        with open(out, "r", encoding="utf-8") as f:
            content = f.read()
        self.assertIn("svg", content.lower())

        self.cs.free(s)
        self._cleanup(out)

    def test_export_pdf_basic(self):
        out = self._out("test_export_pdf_basic.pdf")
        self._cleanup(out)
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        res = self.cs.export_pdf(s, out, 800, 600)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 1000)

        # PDF should start with %PDF header
        with open(out, "rb") as f:
            header = f.read(5)
        self.assertEqual(header, b"%PDF-")

        self.cs.free(s)
        self._cleanup(out)

    def test_export_gif_basic(self):
        out = self._out("test_export_gif_basic.gif")
        self._cleanup(out)
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        res = self.cs.export_gif(s, out, 400, 300)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 500)

        # GIF should start with GIF header
        with open(out, "rb") as f:
            header = f.read(6)
        self.assertIn(header[:3], [b"GIF"])

        self.cs.free(s)
        self._cleanup(out)

    def test_export_png_second_slide(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "features.md")
        )
        out = self._out("test_export_slide2.png")
        self._cleanup(out)
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreater(count, 1)

        res = self.cs.export_png(s, 1, out, 800, 600)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 1000)

        self.cs.free(s)
        self._cleanup(out)

    def test_export_with_different_themes(self):
        out = self._out("test_export_theme_export.png")
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        for theme in ["light", "dracula", "nord"]:
            with self.subTest(theme=theme):
                self.cs.set_theme(s, theme)
                self._cleanup(out)
                res = self.cs.export_png(s, 0, out, 400, 300)
                self.assertEqual(res, 0)
                self.assertTrue(os.path.exists(out))
                self.assertGreater(os.path.getsize(out), 500)

        self.cs.free(s)
        self._cleanup(out)

    def test_export_svg_all_slides(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        for i in range(count):
            out = self._out(f"test_svg_slide{i}.svg")
            self._cleanup(out)
            res = self.cs.export_svg(s, i, out, 800, 600)
            self.assertEqual(res, 0)
            self.assertTrue(os.path.exists(out))
            self.assertGreater(os.path.getsize(out), 100)
            self._cleanup(out)

        self.cs.free(s)

    def test_export_formats_with_all_themes(self):
        themes = ["dark", "light", "dracula", "monokai", "nord"]
        s = self.cs.load(self.md_path)
        self.assertTrue(bool(s))

        for theme in themes:
            with self.subTest(theme=theme):
                self.cs.set_theme(s, theme)

                out_png = self._out(f"test_fmt_{theme}.png")
                self._cleanup(out_png)
                self.assertEqual(self.cs.export_png(s, 0, out_png, 400, 300), 0)
                self._cleanup(out_png)

                out_jpg = self._out(f"test_fmt_{theme}.jpg")
                self._cleanup(out_jpg)
                self.assertEqual(self.cs.export_jpg(s, 0, out_jpg, 400, 300, 85), 0)
                self._cleanup(out_jpg)

                out_svg = self._out(f"test_fmt_{theme}.svg")
                self._cleanup(out_svg)
                self.assertEqual(self.cs.export_svg(s, 0, out_svg, 400, 300), 0)
                self._cleanup(out_svg)

        self.cs.free(s)


if __name__ == "__main__":
    unittest.main()
