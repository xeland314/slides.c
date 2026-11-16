import unittest
import os
import tempfile
from c_slides_bindings import CSlides, LineType


class TestAutosplit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode="w", suffix=".md", delete=False,
                                        encoding="utf-8", dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        self.addCleanup(os.unlink, f.name)
        return f.name

    def _load(self, content):
        s = self.cs.load(self._tmp_md(content))
        self.assertTrue(bool(s))
        self.addCleanup(self.cs.free, s)
        return s

    def _lines(self, s, slide):
        out = []
        for i in range(self.cs.test_get_nlines(s, slide)):
            out.append((self.cs.test_get_line_type(s, slide, i),
                        self.cs.test_get_line_text(s, slide, i)))
        return out

    def test_one_slide_per_heading(self):
        s = self._load("# A\n\ntexto\n\n## B\n\notro\n\n## C\n\nfin\n")
        self.assertEqual(self.cs.get_count(s), 3)
        self.assertEqual(self.cs.test_get_auto_split(s), 1)
        self.assertEqual(self.cs.test_get_line_type(s, 0, 0), LineType.TITLE)
        self.assertEqual(self.cs.test_get_line_type(s, 1, 0), LineType.SUBTITLE)
        self.assertEqual(self.cs.test_get_line_text(s, 1, 0), "B")
        self.assertEqual(self.cs.test_get_line_text(s, 2, 0), "C")

    def test_leading_heading_no_empty_slide(self):
        s = self._load("\n\n# Titulo\n\ntexto\n")
        self.assertEqual(self.cs.get_count(s), 1)
        self.assertEqual(self.cs.test_get_auto_split(s), 1)
        types = [t for t, _ in self._lines(s, 0) if t != LineType.EMPTY]
        self.assertEqual(types[0], LineType.TITLE)

    def test_long_content_splits_with_cont_subtitle(self):
        parts = ["# Guia", ""]
        for i in range(1, 15):
            parts += ["parrafo %d" % i, ""]
        s = self._load("\n".join(parts) + "\n")
        self.assertGreaterEqual(self.cs.get_count(s), 2)
        self.assertEqual(self.cs.test_get_line_type(s, 1, 0), LineType.SUBTITLE)
        self.assertEqual(self.cs.test_get_line_text(s, 1, 0), "Guia (cont.)")
        self.assertEqual(self.cs.test_get_line_text(s, 1, 1), "parrafo 12")
        # Ninguna diapositiva conserva más de AUTOSPLIT_MAX_CONTENT contenidos antes del corte
        self.assertEqual(self.cs.test_get_line_text(s, 0, 0), "Guia")

    def test_long_content_without_heading_has_no_cont_subtitle(self):
        parts = []
        for i in range(1, 16):
            parts += ["linea %d" % i, ""]
        s = self._load("\n".join(parts))
        self.assertGreaterEqual(self.cs.get_count(s), 2)
        self.assertNotEqual(self.cs.test_get_line_type(s, 1, 0), LineType.SUBTITLE)

    def test_code_block_not_split_under_limit(self):
        code = ["```python"] + ["x = %d" % i for i in range(8)] + ["```"]
        s = self._load("# Codigo\n\n" + "\n".join(code) + "\n")
        self.assertEqual(self.cs.get_count(s), 1)
        types = [t for t, _ in self._lines(s, 0)]
        self.assertIn(LineType.CODE_START, types)
        self.assertIn(LineType.CODE_END, types)

    def test_heading_inside_code_not_split(self):
        s = self._load("# X\n\n```\n## no es titulo\n```\n")
        self.assertEqual(self.cs.get_count(s), 1)

    def test_table_not_broken(self):
        # Llegar a 12 contenidos, luego una línea vacía: el corte cae antes de la tabla
        parts = ["# Tabla", ""]
        for i in range(1, 12):
            parts += ["p%d" % i, ""]
        parts += ["| a | b |", "|---|---|", "| 1 | 2 |", "| 3 | 4 |",
                  "| 5 | 6 |", "| 7 | 8 |"]
        s = self._load("\n".join(parts) + "\n")
        n = self.cs.get_count(s)
        self.assertGreaterEqual(n, 2)
        rows_per_slide = []
        for si in range(n):
            rows = sum(1 for t, _ in self._lines(s, si)
                       if t in (LineType.TABLE_ROW, LineType.TABLE_SEP))
            rows_per_slide.append(rows)
        self.assertEqual(sorted(rows_per_slide), [0] * (n - 1) + [6])

    def test_separators_file_unchanged(self):
        s = self._load("# A\n\ntexto\n\n---\n\n# B\n## C\n")
        self.assertEqual(self.cs.get_count(s), 2)
        self.assertEqual(self.cs.test_get_auto_split(s), 0)
        # "## C" no debe dividir: el archivo tiene separadores
        self.assertEqual(self.cs.test_get_line_type(s, 1, 2), LineType.SUBTITLE)

    def test_frontmatter_autosplit_false_disables(self):
        for val in ("false", "off", "no"):
            with self.subTest(val=val):
                s = self._load("---\nautosplit: %s\n---\n# A\n\n# B\n\n## C\n" % val)
                self.assertEqual(self.cs.get_count(s), 1)
                self.assertEqual(self.cs.test_get_auto_split(s), 0)

    def test_frontmatter_autosplit_auto_enables(self):
        for val in ("auto", "true", "on", "yes"):
            with self.subTest(val=val):
                s = self._load("---\nautosplit: %s\n---\n# A\n\n# B\n" % val)
                self.assertEqual(self.cs.get_count(s), 2)
                self.assertEqual(self.cs.test_get_auto_split(s), 1)

    def test_frontmatter_without_separators_works(self):
        s = self._load("---\ntheme: dark\nscale: 1.2\n---\n# A\n\n# B\n\ntexto\n")
        self.assertEqual(self.cs.get_count(s), 2)
        self.assertEqual(self.cs.test_get_auto_split(s), 1)
        self.assertAlmostEqual(self.cs.get_font_scale(s), 1.2)

    def test_hard_limit_inside_code_closes_and_reopens(self):
        code = ["```python"] + ["y = %d" % i for i in range(200)] + ["```"]
        s = self._load("\n".join(code) + "\n")
        n = self.cs.get_count(s)
        self.assertGreaterEqual(n, 2)
        last = self.cs.test_get_nlines(s, 0) - 1
        self.assertEqual(self.cs.test_get_line_type(s, 0, last), LineType.CODE_END)
        self.assertEqual(self.cs.test_get_line_type(s, 1, 0), LineType.CODE_START)
        self.assertEqual(self.cs.test_get_line_text(s, 1, 0), "python")

    def test_title_with_subtitle_stays_together(self):
        s = self._load("# Portada\n\n## Subtitulo\n\ntexto\n\n## Seccion\n\nmas\n")
        self.assertEqual(self.cs.get_count(s), 2)
        types = [t for t, _ in self._lines(s, 0)]
        self.assertIn(LineType.TITLE, types)
        self.assertIn(LineType.SUBTITLE, types)

    def test_dense_list_without_blank_lines_splits(self):
        items = "\n".join("- item %d" % i for i in range(40))
        s = self._load("# Lista\n" + items + "\n")
        self.assertGreaterEqual(self.cs.get_count(s), 3)
        self.assertEqual(self.cs.test_get_line_text(s, 1, 0), "Lista (cont.)")
        for i in range(self.cs.get_count(s)):
            self.assertLessEqual(self.cs.test_get_nlines(s, i), 20)


if __name__ == "__main__":
    unittest.main()
