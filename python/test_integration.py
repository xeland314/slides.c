import unittest
import os
from c_slides_bindings import CSlides

class TestIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "libslider.so"))
        cls.cs = CSlides(lib_path)

    def test_slider_load_basic(self):
        # Asumimos que examples/basic.md existe en la raíz
        path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "examples", "basic.md"))
        s = self.cs.load(path)
        self.assertTrue(bool(s), "No se pudo cargar el slider")
        
        count = self.cs.get_count(s)
        self.assertEqual(count, 2)
        
        # Verificar tema por defecto
        theme_name = self.cs.get_theme_name(s)
        self.assertEqual(theme_name, "dark")
        
        self.cs.free(s)

    def test_slider_change_palette(self):
        path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "examples", "basic.md"))
        s = self.cs.load(path)
        self.assertTrue(bool(s))
        
        self.cs.set_theme(s, "rose")
        theme_name = self.cs.get_theme_name(s)
        self.assertEqual(theme_name, "rose")
        
        self.cs.free(s)

if __name__ == "__main__":
    unittest.main()
