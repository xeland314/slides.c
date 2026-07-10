import unittest
import os
import random
import string
from c_slides_bindings import CSlides

class TestMarkup(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "libslider.so"))
        cls.cs = CSlides(lib_path)

    def test_simple_bold(self):
        out = self.cs.md_to_markup("**bold**")
        self.assertEqual(out, "<b>bold</b>")

    def test_simple_italic(self):
        out = self.cs.md_to_markup("*italic*")
        self.assertEqual(out, "<i>italic</i>")

    def test_combined_bold_italic(self):
        out = self.cs.md_to_markup("***ambos***")
        self.assertEqual(out, "<b><i>ambos</i></b>")

    def test_nested_complex(self):
        out = self.cs.md_to_markup("Texto **con bold y *italic* mixto**")
        self.assertEqual(out, "Texto <b>con bold y <i>italic</i> mixto</b>")

    def test_escaping(self):
        out = self.cs.md_to_markup("A & B < C")
        self.assertEqual(out, "A &amp; B &lt; C")

    def test_inline_code(self):
        out = self.cs.md_to_markup("Usa `printf()`")
        self.assertEqual(out, "Usa <tt>printf()</tt>")

        out = self.cs.md_to_markup("`**no bold**`")
        self.assertEqual(out, "<tt>**no bold**</tt>")

    def test_fuzzing_markup(self):
        tokens = ["*", "**", "***", "_", "__", "___", "&", "<", ">", "`", " ", "abc", "123"]
        
        for i in range(10000):
            input_str = "".join(random.choice(tokens) for _ in range(random.randint(0, 20)))
            output = self.cs.md_to_markup(input_str)
            
            # Verificar tags válidos
            p = 0
            while p < len(output):
                if output[p] == '<':
                    valid_tags = ["<b>", "<i>", "<tt>", "</b>", "</i>", "</tt>"]
                    found = False
                    for tag in valid_tags:
                        if output[p:].startswith(tag):
                            found = True
                            break
                    self.assertTrue(found, f"Tag inválido generado en iteración {i}: {output[p:]} (Input: {input_str})")
                
                if output[p] == '&':
                    valid_escapes = ["&amp;", "&lt;", "&gt;"]
                    found = False
                    for esc in valid_escapes:
                        if output[p:].startswith(esc):
                            found = True
                            break
                    self.assertTrue(found, f"Ampersand no escapado en iteración {i}: {output[p:]} (Input: {input_str})")
                p += 1

if __name__ == "__main__":
    unittest.main()
