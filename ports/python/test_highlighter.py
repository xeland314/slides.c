import unittest
import os
import sys
from c_slides_bindings import CSlides

class TestHighlighter(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Asumimos que libslider.so está en la raíz del proyecto
        lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "libslider.so"))
        cls.cs = CSlides(lib_path)
        cls.theme = cls.cs.theme_default()

    def test_highlighter_basic(self):
        # Keyword
        out = self.cs.highlight("if", self.theme)
        self.assertIn("if", out)
        self.assertIn("<span foreground=", out)

        # Comment
        out = self.cs.highlight("// comment", self.theme)
        self.assertIn("comment", out)
        self.assertIn("<span foreground=", out)

        # Symbols
        out = self.cs.highlight("{}", self.theme)
        self.assertIn("{", out)
        self.assertIn("}", out)
        self.assertIn("<span foreground=", out)

    def test_highlighter_escaping(self):
        # Escaping
        out = self.cs.highlight("a < b && c > d", self.theme)
        self.assertIn("&lt;", out)
        self.assertIn("&gt;", out)
        self.assertIn("&amp;", out)

    def test_highlighter_string(self):
        # String
        out = self.cs.highlight("\"hello world\"", self.theme)
        self.assertIn("hello world", out)
        self.assertIn("<span foreground=", out)

if __name__ == "__main__":
    unittest.main()
